#include <stdio.h>
#include <unistd.h>

#include "image.h"
#include "model.h"
#include "sdk_common.h"
#include "sz_face_module.h"
#include "sz_image_module.h"

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

  //加载模型
  int modelLen = 0;
  unsigned char *pModelData = loadModel(modelFile, &modelLen);
  SZ_HANDLE faceHandle = NULL;
  SZ_RETCODE ret = gszFace.createHandle(pModelData, modelLen, &faceHandle);
  free(pModelData);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] gszFace.createHandle failed !\n");
    return -1;
  }

  SZ_HANDLE imgHandle = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_FACE_FEATURE *pFeature = NULL;
  SZ_FACE_QUALITY quality;
  SZ_INT32 featureLen = 0;
  int faceCnt = 0;
  SZ_FACE_FEATURE *pfeatureList[IMG_NUM];
  float scores[IMG_NUM][IMG_NUM] = {1.0};

  for (int i = 0; i < IMG_NUM; i++) {
    printf("[INFO] jpgFiles: %d %s \n", i, jpgFiles[i]);

    // **********
    // 读入人脸照片,然后得到人脸特征值
    // **********
    bOk = getImageFromjpg(jpgFiles[i], &imgHandle);
    if (!bOk) goto JUMP;

    ret = gszFace.detect(faceHandle, imgHandle, &faceCnt);
    if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
      printf("[ERR] gszFace.detect failed !\n");
      goto JUMP;
    }

    ret = gszFace.evaluate(faceHandle, imgHandle, 0, &quality);
    printf("[INFO] face quality: (%f, %f, %f, %f, %f, %f)\n", quality.pitch,
           quality.yaw, quality.roll, quality.leftScore, quality.rightScore,
           quality.mouthScore);

    // 获取索引0的脸的特征
    ret = gszFace.extractFeatureByIndex(faceHandle, imgHandle, 0, &pFeature,
                                        &featureLen);
    //保存抽取的特征，防止特征被覆盖
    pfeatureList[i] = (SZ_FACE_FEATURE *)malloc(featureLen);
    memcpy(pfeatureList[i], pFeature, featureLen);
  }

  printf("==============================\n");
  printf("======all compare scores=======\n");
  for (int i = 0; i < IMG_NUM; i++) {
    for (int j = i + 1; j < IMG_NUM; j++) {
      ret = gszFace.compareFeature(faceHandle, pfeatureList[i], pfeatureList[j],
                                   &compareScore);
      if (ret != SZ_RETCODE_OK) {
        printf("[ERR] gszFace.compareFeature failed at %s %s!\n", jpgFiles[i],
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
  gszImage.releaseHandle(imgHandle);
  gszFace.releaseHandle(faceHandle);
  for (int i = 0; i < IMG_NUM; i++) free(pfeatureList[i]);
  return ret;
}
