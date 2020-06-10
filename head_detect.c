#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "sdk_common.h"
#include "sz_head_module.h"
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
  SZ_BOOL bOk = SZ_FALSE;
  SZ_LICENSE_CTX* licenseCtx = NULL;
  SZ_HEAD_CTX* headCtx = NULL;
  SZ_IMAGE_CTX* imgCtx = NULL;
  SZ_HEAD_DETECTION_LIST detectResult;

  SZ_HEAD_CONFIG config;
  config.prob_threshold = 0.4;
  for (int i = 0; i < K_HEAD_MODEL_N; i++) {
    config.modelToLoad[i] = 1;
  }

  ret = init_handles_head(modelFile, &headCtx, &licenseCtx, &config);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }

  // ********************
  // 读入一张jpg人脸照片
  // ********************
  int width = 0, height = 0;
  unsigned char* pBgrData = readImage(jpgFile, &width, &height);
  if (pBgrData == NULL) {
    printf("[ERR] Read jpgFile %s failed!\n", jpgFile);
    goto JUMP;
  } else {
    printf("[INFO] Read jpgFile %s \n", jpgFile);
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
  // head detect and save results to File
  // ********************
  struct timespec start, end;
  long spend;
  int iModel = 0;
  clock_gettime(0, &start);
  ret = SZ_HEAD_detect(headCtx, imgCtx, &detectResult, iModel);
  clock_gettime(0, &end);
  spend = (end.tv_sec - start.tv_sec) * 1000 +
          (end.tv_nsec - start.tv_nsec) / 1000000;
  printf("\n[head detection]===== TIME SPEND: %ld ms =====\n", spend);
  if (ret != SZ_RETCODE_OK || detectResult.count <= 0) {
    printf("[ERR] SZ_HEAD_detect failed !\n");
  } else {
    printf("SZ_HEAD_detect %d !\n", detectResult.count);
    saveDetection2File_head(jpgFile, (const char*)pBgrData, width, height,
                            iModel, detectResult);
  }

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  SZ_HEAD_CTX_release(headCtx);
  return ret;
}
