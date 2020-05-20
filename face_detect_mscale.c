#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
  SZ_LICENSE_CTX* licenseCtx = NULL;
  SZ_FACE_CTX* faceCtx = NULL;
  ret = init_handles(modelFile, &faceCtx, &licenseCtx);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }

  //创建一个图像句柄
  SZ_IMAGE_CTX* imgCtx = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_FACE_FEATURE* pFeature = NULL;
  SZ_FACE_QUALITY quality;
  SZ_INT32 featureLen = 0;
  int faceCnt = 0;
  SZ_FACE_DETECTION pFaceInfos[128];

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
  // face detect in 1080P and save results to File
  // ********************
  struct timespec start, next, end;
  long spend;
  clock_gettime(0, &start);
  ret = SZ_FACE_detectAndGetInfo_1080P(faceCtx, imgCtx, pFaceInfos, &faceCnt);
  clock_gettime(0, &end);
  spend = (end.tv_sec - start.tv_sec) * 1000 +
          (end.tv_nsec - start.tv_nsec) / 1000000;
  printf("\n[face detection]===== TIME SPEND: %ld ms =====\n", spend);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    printf("[ERR] SZ_FACE_detect failed !\n");
  } else {
    saveDetection2File(jpgFile, pBgrData, width, height, 1080, faceCnt,
                       pFaceInfos);
  }

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  SZ_FACE_CTX_release(faceCtx);
  return ret;
}
