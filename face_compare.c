#include <stdio.h>
#include <unistd.h>

#include "image.h"
#include "model.h"
#include "sdk_common.h"
#include "sz_face_module.h"
#include "sz_image_module.h"

static void usage(char **argv) {
  printf(
      "usage: %s [a:b:c]  \n"
      "       -a  face1.jpg\n"
      "       -b  face2.jpg\n"
      "       -c  facemodel\n",
      argv[0]);
  printf(
      "for example:\n"
      "%s -a face1.jpg -b face2.jpg -c facemodel\n",
      argv[0]);
}

int main(int argc, char **argv) {
  char jpgFile1[MAX_FILE_LEN + 1] = {0};
  char jpgFile2[MAX_FILE_LEN + 1] = {0};
  char modelFile[MAX_FILE_LEN + 1] = {0};
  if (argc < 7) {
    usage(argv);
    return -1;
  }

  //解析参数
  int bInvaildParam = 0;
  int c;
  while ((c = getopt(argc, argv, "a:b:c:")) != EOF) {
    switch (c) {
      case 'a':
        strncpy(jpgFile1, optarg, MAX_FILE_LEN);
        printf("[INFO] input image path1: %s \n", jpgFile1);
        break;
      case 'b':
        strncpy(jpgFile2, optarg, MAX_FILE_LEN);
        printf("[INFO] input image path2: %s \n", jpgFile2);
        break;
      case 'c':
        strncpy(modelFile, optarg, MAX_FILE_LEN);
        printf("[INFO] input model path:  %s \n", modelFile);
        break;
      default:
        bInvaildParam = 1;
        break;
    }
  }
  if (bInvaildParam || strlen(jpgFile1) == 0 || strlen(jpgFile1) == 0 ||
      strlen(modelFile) == 0 ||
      (strstr(jpgFile1, ".jpg") == NULL && strstr(jpgFile1, ".jpeg") == NULL &&
       strstr(jpgFile1, ".png") == NULL) ||
      (strstr(jpgFile2, ".jpg") == NULL && strstr(jpgFile2, ".jpeg") == NULL &&
       strstr(jpgFile2, ".png") == NULL)) {
    usage(argv);
    return -1;
  }

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

  // **********
  // 读入第一张jpg人脸照片,然后得到人脸特征值
  // **********
  bOk = getImageFromjpg(jpgFile1, &imgHandle);
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
  SZ_FACE_FEATURE *pFeature1 = (SZ_FACE_FEATURE *)malloc(featureLen);
  memcpy(pFeature1, pFeature, featureLen);

  // **********
  // 读入第二张jpg人脸照片,然后得到人脸特征值
  // **********
  bOk = getImageFromjpg(jpgFile2, &imgHandle);
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

  // 获取索引为0的脸的特征
  ret = gszFace.extractFeatureByIndex(faceHandle, imgHandle, 0, &pFeature,
                                      &featureLen);
  //保存抽取的特征，防止特征被覆盖
  SZ_FACE_FEATURE *pFeature2 = (SZ_FACE_FEATURE *)malloc(featureLen);
  memcpy(pFeature2, pFeature, featureLen);

  ret = gszFace.compareFeature(faceHandle, pFeature1, pFeature2, &compareScore);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] gszFace.compareFeature failed !\n");
    goto JUMP;
  }
  printf("=============\n");
  printf("[INFO] img1[%s] Vs img2[%s]: face similarity score = %f\n", jpgFile1,
         jpgFile2, compareScore);
  printf("=============\n");

JUMP:
  gszImage.releaseHandle(imgHandle);
  gszFace.releaseHandle(faceHandle);
  free(pFeature1);
  return ret;
}
