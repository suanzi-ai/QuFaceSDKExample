#include "sdk_common.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "opencv2/opencv.hpp"
#include "sz_face_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

void saveDetection2File(const char *jpgFile, const char *pData, int w, int h,
                        int scaleN, int faceCnt, SZ_FACE_CTX *faceCtx) {
  std::string saveName =
      std::string(jpgFile) + "_scale_" + std::to_string(scaleN) + ".jpg";

  cv::Mat image(h, w, CV_8UC3, (void *)pData);

  SZ_FACE_DETECTION faceInfo;
  SZ_RETCODE ret;
  for (SZ_INT32 idx = 0; idx < faceCnt; idx++) {
    ret = SZ_FACE_getDetectInfo(faceCtx, idx, &faceInfo);
    if (ret != SZ_RETCODE_OK) {
      printf("[ERR] SZ_FACE_getDetectInfo(%d) failed!\n", idx);
      continue;
    }

    printf("Scale-%d: Face-%d [x=%d, y=%d, w=%d, h=%d]\n", scaleN, idx,
           faceInfo.rect.x, faceInfo.rect.y, faceInfo.rect.width,
           faceInfo.rect.height);

    cv::rectangle(image,
                  cv::Rect(faceInfo.rect.x, faceInfo.rect.y,
                           faceInfo.rect.width, faceInfo.rect.height),
                  cv::Scalar(255, 0, 0), 2);
  }

  cv::imwrite(saveName.c_str(), image);
}

unsigned char *loadModel(const char *modelFile, int *pModelLen) {
  FILE *fp = fopen(modelFile, "r");
  if (fp == NULL) {
    printf("[ERR] Model file %s not exists\n", modelFile);
    return NULL;
  }

  //获取模型文件长度
  fseek(fp, 0L, SEEK_END);
  int fileLen = 0;
  if ((fileLen = ftell(fp)) == -1) {
    fclose(fp);
    printf("[ERR] Read file %s len failed\n", modelFile);
    return NULL;
  }

  //分配模型缓冲
  unsigned char *pModelData = (unsigned char *)malloc(fileLen);
  if (pModelData == NULL) {
    fclose(fp);
    printf("[ERR] Alloc mem failed\n");
    return NULL;
  }

  printf("[INFO] loadModel: model filelen = %d\n", fileLen);
  //读模型数据到模型缓冲pModelData
  rewind(fp);  // fseek(fp, 0L, SEEK_SET)的另一种写法
  *pModelLen = fread(pModelData, 1, fileLen, fp);
  if ((*pModelLen) != fileLen) {
    fclose(fp);
    printf("[ERR] Read file %s failed\n", modelFile);
    return NULL;
  }

  fclose(fp);
  return pModelData;
}

SZ_BOOL getFeature(const char *jpgFile, SZ_FACE_CTX *faceCtx,
                   SZ_IMAGE_CTX *imgCtx, SZ_FACE_FEATURE **pFeature,
                   SZ_INT32 *pFeatureLen) {
  int width = 0, height = 0;
  unsigned char *pBgrData = jpg2Bgr(jpgFile, &width, &height);
  if (pBgrData == NULL) return SZ_FALSE;

  //更新图像句柄尺寸
  int faceCnt = 0;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_RETCODE ret = SZ_IMAGE_setData(imgCtx, pBgrData, width, height);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }

#if 1

  ret = SZ_FACE_detect(faceCtx, imgCtx, &faceCnt);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    goto JUMP;
  }

  SZ_FACE_QUALITY quality;
  ret = SZ_FACE_evaluate(faceCtx, imgCtx, 0, &quality);
  printf("[INFO] getFeature(): face quality(%f, %f, %f, %f, %f, %f)\n",
         quality.pitch, quality.yaw, quality.roll, quality.leftScore,
         quality.rightScore, quality.mouthScore);

  //始终获取索引为0的脸的特征
  ret =
      SZ_FACE_extractFeatureByIndex(faceCtx, imgCtx, 0, pFeature, pFeatureLen);
#else
  SZ_RECT rect = {0, 0, width, height};
  printf("%d %d %d %d \n", rect.x, rect.y, rect.width, rect.height);
  ret = SZ_FACE_extractFeatureByPosition(faceCtx, imgCtx, &rect, pFeature,
                                         pFeatureLen);
#endif
  if (ret != SZ_RETCODE_OK) goto JUMP;
  bOk = SZ_TRUE;
JUMP:
  free(pBgrData);
  return bOk;
}

SZ_BOOL getImageFromjpg(const char *jpgFile, SZ_IMAGE_CTX **pImgCtx) {
  int width = 0, height = 0;
  SZ_RETCODE ret;
  SZ_BOOL bOk = SZ_FALSE;

  unsigned char *pBgrData = jpg2Bgr(jpgFile, &width, &height);
  if (pBgrData == NULL) {
    printf("[ERR] getImageFromjpg: read jpgFile %s failed!\n", jpgFile);
    return SZ_FALSE;
  }

  if (*pImgCtx == NULL) {
    *pImgCtx = SZ_IMAGE_CTX_create(width, height, SZ_IMAGETYPE_BGR);
    if (*pImgCtx == NULL) {
      printf("[ERR] getImageFromjpg: creat imageCtx failed!\n");
      goto JUMP;
    }
  }

  ret = SZ_IMAGE_setData(*pImgCtx, pBgrData, width, height);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] getImageFromjpg: setData failed!\n");
    goto JUMP;
  }

  bOk = SZ_TRUE;
JUMP:
  free(pBgrData);
  return bOk;
}

SZ_RETCODE init_handles(const char *modelFile, SZ_FACE_CTX **pFaceCtx,
                        SZ_LICENSE_CTX **pLicenseCtx) {
  SZ_NET_CTX *netCtx = NULL;
  if (init_handles_ex(modelFile, pFaceCtx, pLicenseCtx, &netCtx) !=
      SZ_RETCODE_OK) {
    return SZ_RETCODE_FAILED;
  }
  SZ_NET_detach(netCtx);
  SZ_NET_CTX_release(netCtx);
  return SZ_RETCODE_OK;
}

SZ_RETCODE init_handles_ex(const char *modelFile, SZ_FACE_CTX **pFaceCtx,
                           SZ_LICENSE_CTX **pLicenseCtx, SZ_NET_CTX **pNetCtx) {
  SZ_RETCODE ret;
  NetCreateOption opts = {0};
  opts.storagePath = (char *)".";
  opts.clientId = (char *)"QufaceHisiDemo";

  std::ifstream i("device_info.json");
  if (!i.is_open()) {
    printf("[ERR] device_info.json not present\n");
    return SZ_RETCODE_FAILED;
  }

  nlohmann::json device_info;
  i >> device_info;

  if (!device_info.contains("ProductKey") ||
      !device_info.contains("DeviceName") ||
      !device_info.contains("DeviceSecret") ||
      device_info["ProductKey"].empty() || device_info["DeviceName"].empty() ||
      device_info["DeviceSecret"].empty()) {
    printf(
        "[ERR] field ProductKey or DeviceName or DeviceSecret is missing in "
        "%s\n",
        "device_info.json");
    return SZ_RETCODE_FAILED;
  }
  std::string productKey = device_info["ProductKey"];
  std::string deviceName = device_info["DeviceName"];
  std::string deviceSecret = device_info["DeviceSecret"];
  opts.productKey = (char *)productKey.c_str();
  opts.deviceName = (char *)deviceName.c_str();
  opts.deviceSecret = (char *)deviceSecret.c_str();

  *pNetCtx = SZ_NET_CTX_create(opts);
  if (*pNetCtx == NULL) {
    printf("[ERR] SZ_NET_CTX_create failed !\n");
    return SZ_RETCODE_FAILED;
  }

  ret = SZ_NET_connect(*pNetCtx);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] SZ_NET_connect failed !\n");
    return ret;
  }

  *pLicenseCtx = SZ_LICENSE_CTX_create(*pNetCtx);
  if (*pLicenseCtx == NULL) {
    printf("[ERR] SZ_LICENSE_CTX_create failed !\n");
    return SZ_RETCODE_FAILED;
  }

  ret = SZ_LICENSE_auth(*pLicenseCtx);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] SZ_LICENSE_auth failed !\n");
    return ret;
  }

  //加载模型
  int modelLen = 0;
  unsigned char *pModelData = loadModel(modelFile, &modelLen);
  *pFaceCtx = SZ_FACE_CTX_create(*pLicenseCtx, pModelData, modelLen);
  free(pModelData);
  if (*pFaceCtx == NULL) {
    printf("[ERR] SZ_FACE_CTX_create failed !\n");
    return SZ_RETCODE_FAILED;
  }
  return SZ_RETCODE_OK;
}

unsigned char *jpg2Bgr(const char *jpgFile, int *pWidth, int *pHeight) {
  cv::Mat img = cv::imread(jpgFile);
  if (img.empty()) {
    printf("%s is not exist!\n", jpgFile);
    return NULL;
  }

  // cv::resize(img, img, {200,200});

  int width = img.cols;
  int height = img.rows;

  //返回图像宽度和高度
  *pWidth = width;
  *pHeight = height;

  //分配存放BGR图像数据缓冲
  unsigned char *pData = (unsigned char *)malloc(width * height * 3);
  memcpy(pData, img.ptr(), width * height * 3);

  //返回BGR图像数据缓冲
  return pData;
}

unsigned char *jpg2Nv21(const char *jpgFile, int *pWidth, int *pHeight) {
  cv::Mat img = cv::imread(jpgFile);
  if (img.empty()) {
    printf("%s is not exist!\n", jpgFile);
    return NULL;
  }

  int width = img.cols;
  int height = img.rows;

  //分配存放NV21图像数据缓冲
  unsigned char *pData = (unsigned char *)malloc(width * height * 3 / 2);

  cv::Mat yuv420pImg;
  // bgr 转 yuv420p
  cvtColor(img, yuv420pImg, cv::COLOR_BGR2YUV_I420);

  //返回图像宽度和高度
  *pWidth = width;
  *pHeight = height;

  // yuv420p 转 nv21
  int len = width * height;
  memcpy(pData, yuv420pImg.ptr(), len);
  unsigned char *p = pData + len;
  unsigned char *u = yuv420pImg.ptr() + len;
  unsigned char *v = yuv420pImg.ptr() + len + len / 4;
  for (int i = 0; i < len / 4; i++) {
    p[i * 2] = v[i];
    p[i * 2 + 1] = u[i];
  }

  //返回NV21图像缓冲数据
  return pData;
}

unsigned char *readImage(const char *jpgFile, int *pWidth, int *pHeight) {
  cv::Mat img = cv::imread(jpgFile);
  if (img.empty()) {
    printf("Read %s Failed!\n", jpgFile);
    return NULL;
  }

  int width = img.cols;
  int height = img.rows;

  //分配存放BGR图像数据缓冲
  unsigned char *pData = (unsigned char *)malloc(width * height * 3);
  memcpy(pData, img.ptr(), width * height * 3);

  //返回BGR图像数据缓冲
  *pWidth = width;
  *pHeight = height;
  return pData;
}
