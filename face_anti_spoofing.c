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

#define IMG_NUM 8
static void usage(char** argv) {
  printf("usage: %s facemodel\n", argv[0]);
  printf(
      "for example:\n"
      "%s  facemodel\n",
      argv[0]);
}

int main(int argc, char** argv) {
  char jpgFile1[MAX_FILE_LEN + 1] = {0};
  char jpgFile2[MAX_FILE_LEN + 1] = {0};
  char modelFile[MAX_FILE_LEN + 1] = {0};

  //解析参数
  if (argc > 1) {
    strncpy(modelFile, argv[1], MAX_FILE_LEN);
    printf("[INFO] input model path:  %s \n", modelFile);
  } else {
    usage(argv);
    return -1;
  }

  char jpgFiles[IMG_NUM][MAX_FILE_LEN + 1] = {
      {"data/lyf1.jpg"},   {"data/lyf2.jpg"},  {"data/lyf3.jpg"},
      {"data/zxy1.jpg"},   {"data/zxy2.jpg"},  {"data/anti-1.jpg"},
      {"data/anti-2.jpg"}, {"data/anti-3.jpg"}};

  SZ_RETCODE ret;
  SZ_LICENSE_CTX* licenseCtx = NULL;
  SZ_FACE_CTX* faceCtx = NULL;
  ret = init_handles(modelFile, &faceCtx, &licenseCtx);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }

  SZ_IMAGE_CTX* imgCtx = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_FACE_FEATURE* pFeature = NULL;
  SZ_FACE_QUALITY quality;
  SZ_INT32 featureLen = 0;
  int faceCnt = 0;

  for (int i = 0; i < IMG_NUM; i++) {

    bOk = getImageFromjpg(jpgFiles[i], &imgCtx);
    if (!bOk) goto JUMP;

    ret = SZ_FACE_detect(faceCtx, imgCtx, &faceCnt);
    if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
      printf("[ERR] SZ_FACE_detect failed !\n");
      goto JUMP;
    }

    clock_t t1, t2;
    SZ_ATTRIBUTE* attributes;
    int len = 0;
    t1 = clock();
    ret = SZ_FACE_getAttributeByIndex(
        faceCtx, imgCtx, 0, SZ_FACE_ATTR_TYPE_LIVENESS, &attributes, &len);
    t2 = clock();
    printf("[INFO] ==== face anti spoofing take time: %f s \n",
           (double)(t2 - t1) / CLOCKS_PER_SEC);
    if (len > 0) {
      if (attributes[0].value > 0)
        printf("[INFO] ==== jpgFiles %s is real face ====\n", jpgFiles[i]);
      else {
        printf("[INFO] ==== jpgFiles %s is fake face !!! ====\n", jpgFiles[i]);
      }
    }
  }

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  SZ_FACE_CTX_release(faceCtx);
  return ret;
}
