#include "sdk_common.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "image.h"
#include "model.h"
#include "opencv2/opencv.hpp"
#include "sz_face_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

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
                        SZ_LICENSE_CTX **pLicenseCtx, SZ_NET_CTX **pNetCtx) {
  SZ_RETCODE ret;
  NetCreateOption opts = {0};
  opts.storagePath = (char *)".";
  opts.clientId = (char *)"QufaceHisiDemo";

  std::ifstream i("device_info.json");
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
