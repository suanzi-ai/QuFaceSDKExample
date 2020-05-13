#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sdk_common.h"
#include "sz_face_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

static void usage(char** argv) {
  printf(
      "usage: %s [a:b]  \n"
      "       -a  face.jpg\n"
      "       -b  facemodel\n",
      argv[0]);
  printf(
      "for example:\n"
      "%s -a face.jpg -b facemodel\n",
      argv[0]);
}

int main(int argc, char** argv) {
  char jpgFile[MAX_FILE_LEN + 1] = {0};
  char modelFile[MAX_FILE_LEN + 1] = {0};
  if (argc < 5) {
    usage(argv);
    return -1;
  }

  //解析参数
  int bInvaildParam = 0;
  int c;
  while ((c = getopt(argc, argv, "a:b:")) != EOF) {
    switch (c) {
      case 'a':
        strncpy(jpgFile, optarg, MAX_FILE_LEN);
        break;
      case 'b':
        strncpy(modelFile, optarg, MAX_FILE_LEN);
        break;
      default:
        bInvaildParam = 1;
        break;
    }
  }
  if (bInvaildParam || strlen(jpgFile) == 0 || strlen(modelFile) == 0 ||
      (strstr(jpgFile, ".jpg") == NULL && strstr(jpgFile, ".jpeg") == NULL &&
       strstr(jpgFile, ".png") == NULL)) {
    usage(argv);
    return -1;
  }

  SZ_RETCODE ret;
  SZ_NET_CTX* netCtx = NULL;
  SZ_LICENSE_CTX* licenseCtx = NULL;
  SZ_FACE_CTX* faceCtx = NULL;
  ret = init_handles(modelFile, &faceCtx, &licenseCtx, &netCtx);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }
  SZ_NET_detach(netCtx);

  //创建一个图像句柄
  SZ_IMAGE_CTX* imgCtx = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_FACE_FEATURE* pFeature = NULL;
  SZ_FACE_QUALITY quality;
  SZ_INT32 featureLen = 0;
  int faceCnt = 0;

  // ********************
  // 读入一张jpg人脸照片
  // ********************
  int width = 0, height = 0;
  unsigned char* pBgrData = readImage(jpgFile, &width, &height);
  if (pBgrData == NULL) {
    printf("[ERR] Read jpgFile %s failed!\n", jpgFile);
    goto JUMP;
  }

  // ********************
  // create image handle
  // ********************
  imgCtx = SZ_IMAGE_CTX_create(width, height, SZ_IMAGETYPE_BGR);
  if (imgCtx == NULL) {
    printf("[ERR] Creat imageCtx failed!\n");
    goto JUMP;
  }
  ret = SZ_IMAGE_setData(imgCtx, pBgrData, width, height);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] setData failed!\n");
    goto JUMP;
  }

  // ********************
  // face detect scaleN=3 and save results to File
  // ********************
  int scaleN = 3;
  ret = SZ_FACE_detect_mscale(faceCtx, imgCtx, scaleN, &faceCnt);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    printf("[ERR] SZ_FACE_detect failed !\n");
  } else {
    saveDetection2File(jpgFile, pBgrData, width, height, scaleN, faceCnt,
                       faceCtx);
  }

  // ********************
  // face detect scaleN=5 and save results to File
  // ********************
  scaleN = 5;
  ret = SZ_FACE_detect_mscale(faceCtx, imgCtx, scaleN, &faceCnt);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    printf("[ERR] SZ_FACE_detect failed !\n");
  } else {
    saveDetection2File(jpgFile, pBgrData, width, height, scaleN, faceCnt,
                       faceCtx);
  }

  // ********************
  // face detect scaleN=7 and save results to File
  // ********************
  scaleN = 7;
  ret = SZ_FACE_detect_mscale(faceCtx, imgCtx, scaleN, &faceCnt);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    printf("[ERR] SZ_FACE_detect failed !\n");
  } else {
    saveDetection2File(jpgFile, pBgrData, width, height, scaleN, faceCnt,
                       faceCtx);
  }

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_NET_CTX_release(netCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  SZ_FACE_CTX_release(faceCtx);
  return ret;
}
