#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sdk_common.h"
#include "sz_face_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

#define IMG_NUM 8
static void usage(char **argv) {
  printf("usage: %s facemodel\n", argv[0]);
  printf(
      "for example:\n"
      "%s  facemodel\n",
      argv[0]);
}

int main(int argc, char **argv) {
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
      {"data/lyf1.jpg"}, {"data/lyf2.jpg"}, {"data/lyf3.jpg"},
      {"data/zxy1.jpg"}, {"data/zxy2.jpg"}, {"data/szj1.jpg"},
      {"data/szj2.jpg"}, {"data/szj3.jpg"}};

  SZ_RETCODE ret;
  SZ_NET_CTX *netCtx = NULL;
  SZ_LICENSE_CTX *licenseCtx = NULL;
  SZ_FACE_CTX *faceCtx = NULL;
  ret = init_handles(modelFile, &faceCtx, &licenseCtx, &netCtx);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }
  SZ_NET_detach(netCtx);

  SZ_IMAGE_CTX *imgCtx = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_FACE_FEATURE *pFeature = NULL;
  SZ_FACE_QUALITY quality;
  SZ_INT32 featureLen = 0;
  int faceCnt = 0;
  SZ_FACE_FEATURE *pfeatureList[IMG_NUM];
  float scores[IMG_NUM][IMG_NUM] = {1.0};

  // **********
  // 读入人脸照片, 提取人脸特征值
  // **********
  for (int i = 0; i < IMG_NUM; i++) {
    printf("[INFO] jpgFiles: %d %s \n", i, jpgFiles[i]);

    bOk = getImageFromjpg(jpgFiles[i], &imgCtx);
    if (!bOk) goto JUMP;

    ret = SZ_FACE_detect_mscale(faceCtx, imgCtx, 3, &faceCnt);
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

    ret = SZ_FACE_evaluate(faceCtx, imgCtx, 0, &quality);
    printf("[INFO] face quality: (%f, %f, %f, %f, %f, %f)\n", quality.pitch,
           quality.yaw, quality.roll, quality.leftScore, quality.rightScore,
           quality.mouthScore);

    // 获取索引0的脸的特征
    ret = SZ_FACE_extractFeatureByIndex(faceCtx, imgCtx, 0, &pFeature,
                                        &featureLen);
    //保存抽取的特征，防止特征被覆盖
    pfeatureList[i] = (SZ_FACE_FEATURE *)malloc(featureLen);
    memcpy(pfeatureList[i], pFeature, featureLen);
  }

  printf(
      "========================================================================"
      "==================\n");
  printf(
      "====================================all compare "
      "scores====================================\n");
  for (int i = 0; i < IMG_NUM; i++) {
    for (int j = i + 1; j < IMG_NUM; j++) {
      ret = SZ_FACE_compareFeature(faceCtx, pfeatureList[i], pfeatureList[j],
                                   &compareScore);
      if (ret != SZ_RETCODE_OK) {
        printf("[ERR] SZ_FACE_compareFeature failed at %s %s!\n", jpgFiles[i],
               jpgFiles[j]);
      } else {
        // printf(" [%s] vs [%s]:  %0.3f\n\n", jpgFiles[i], jpgFiles[j],
        //       compareScore);
        scores[i][j] = compareScore;
        scores[j][i] = compareScore;
      }
    }
  }

  // display scores in matirx format
  printf("imagleList\t");
  for (int i = 0; i < IMG_NUM; i++) printf("%s\t", jpgFiles[i]);
  printf("\n");
  for (int i = 0; i < IMG_NUM; i++) {
    printf("[%s]\t", jpgFiles[i]);
    for (int j = 0; j < IMG_NUM; j++) {
      if (i == j)
        printf("1.00000000\t");
      else
        printf("%0.8f\t", scores[i][j]);
    }
    printf("\n");
  }

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_NET_CTX_release(netCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  for (int i = 0; i < IMG_NUM; i++) free(pfeatureList[i]);
  return ret;
}
