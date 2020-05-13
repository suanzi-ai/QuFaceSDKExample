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

  // **********
  // 读入一张人脸照片
  // **********
  bOk = getImageFromjpg(jpgFile, &imgCtx);
  if (!bOk) {
    printf("[ERR] read %d image file failed !\n", jpgFile);
    goto JUMP;
  }

  ret = SZ_FACE_detect(faceCtx, imgCtx, &faceCnt);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    printf("[ERR] SZ_FACE_detect failed !\n");
    goto JUMP;
  }

  SZ_FACE_DETECTION faceInfo;
  for (SZ_INT32 idx = 0; idx < faceCnt; idx++) {
    ret = SZ_FACE_getDetectInfo(faceCtx, idx, &faceInfo);
    if (ret != SZ_RETCODE_OK) {
      printf("[ERR] SZ_FACE_getDetectInfo(%d) failed!\n", idx);
      continue;
    }

    printf("Face_%d: [x=%d, y=%d, w=%d, h=%d]\n", idx, faceInfo.rect.x,
           faceInfo.rect.y, faceInfo.rect.width, faceInfo.rect.height);
  }

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_NET_CTX_release(netCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  SZ_FACE_CTX_release(faceCtx);
  return ret;
}
