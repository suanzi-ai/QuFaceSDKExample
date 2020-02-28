#include <stdio.h>
#include <unistd.h>

#include "image.h"
#include "model.h"
#include "sdk_common.h"
#include "sz_database_module.h"
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

typedef struct {
  int age;
  int sex;
  char name[20];
} UserInfo;

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
        break;
      case 'b':
        strncpy(jpgFile2, optarg, MAX_FILE_LEN);
        break;
      case 'c':
        strncpy(modelFile, optarg, MAX_FILE_LEN);
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

  //加载模型并创建人脸算法句柄
  int modelLen = 0;
  unsigned char *pModelData = loadModel(modelFile, &modelLen);
  SZ_HANDLE faceHandle = NULL;
  SZ_RETCODE ret = gszFace.createHandle(pModelData, modelLen, &faceHandle);
  free(pModelData);
  if (ret != SZ_RETCODE_OK) {
    return -1;
  }

  //创建一个图像句柄
  SZ_HANDLE imgHandle = NULL;
  SZ_HANDLE databaseHandle = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_QUERY_RESULT *pQueryResults;
  UserInfo *pCurUserInfo = NULL;
  SZ_INT32 topN = 0;
  SZ_FACE_FEATURE *pFeature = NULL;
  int i = 0;
  ret = gszImage.createHandle(1920, 1080, SZ_IMAGETYPE_BGR, &imgHandle);
  if (ret != SZ_RETCODE_OK) goto JUMP;

  //创建一个人脸检索数据库句柄
  ret = gszDatabase.createHandle("database", &databaseHandle);
  if (ret != SZ_RETCODE_OK) goto JUMP;

  //输入一张jpg人脸照片,然后得到人脸特征值
  SZ_INT32 featureLen = 0;
  bOk = getFeature(jpgFile1, faceHandle, imgHandle, &pFeature, &featureLen);
  if (!bOk) goto JUMP;

  //增加张三人脸特征到database中
  UserInfo userInfo;
  memset(&userInfo, 0x00, sizeof(userInfo));
  userInfo.age = 29;
  userInfo.sex = 0;
  memcpy(userInfo.name, "zhangsan", 9);
  // ret = gszDatabase.add(databaseHandle, 0, pFeature, &userInfo,
  //                     sizeof(UserInfo));
  if (ret != SZ_RETCODE_OK) goto JUMP;

  //输入一张jpg人脸照片,然后得到人脸特征值
  bOk = getFeature(jpgFile2, faceHandle, imgHandle, &pFeature, &featureLen);
  if (!bOk) goto JUMP;

  //增加李四人脸特征到database中
  memset(&userInfo, 0x00, sizeof(userInfo));
  userInfo.age = 23;
  userInfo.sex = 1;
  memcpy(userInfo.name, "lisi", 5);
  // ret = gszDatabase.add(databaseHandle, 1, pFeature, &userInfo,
  //        sizeof(UserInfo));

  // ret = gszDatabase.remove(databaseHandle, 1, &userInfo);
  if (ret != SZ_RETCODE_OK) goto JUMP;
  // printf("remove result = %s   %d   %d\n", userInfo.name, userInfo.age,
  // userInfo.sex);

  //在database中检索张三人脸特征
  ret = gszDatabase.query(databaseHandle, pFeature, &pQueryResults, &topN);
  if (ret != SZ_RETCODE_OK) goto JUMP;

  printf("topN: %d \n", topN);

  //输出检索结果
  for (i = 0; i < topN; i++) {
    pCurUserInfo = (UserInfo *)pQueryResults[i].pInfo;
    printf("the top %d:score = %f, name=%s, age=%d, sex=%d\n", i,
           pQueryResults[i].score, pCurUserInfo->name, pCurUserInfo->age,
           pCurUserInfo->sex);
  }

JUMP:
  gszImage.releaseHandle(imgHandle);
  gszFace.releaseHandle(faceHandle);
  gszDatabase.releaseHandle(databaseHandle);
  return ret;
}
