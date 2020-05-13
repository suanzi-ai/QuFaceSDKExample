#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sdk_common.h"
#include "sz_database_module.h"
#include "sz_face_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

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

  SZ_RETCODE ret;
  SZ_NET_CTX *netCtx = NULL;
  SZ_LICENSE_CTX *licenseCtx = NULL;
  SZ_FACE_CTX *faceCtx = NULL;
  ret = init_handles(modelFile, &faceCtx, &licenseCtx, &netCtx);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }
  SZ_NET_detach(netCtx);

  //创建一个图像句柄
  SZ_IMAGE_CTX *imgCtx = NULL;
  SZ_DATABASE_CTX *databaseCtx = NULL;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_FLOAT compareScore = 0.0;
  SZ_QUERY_RESULT *pQueryResults;
  UserInfo *pCurUserInfo = NULL;
  SZ_INT32 topN = 0;
  SZ_FACE_FEATURE *pFeature = NULL;
  int i = 0;
  imgCtx = SZ_IMAGE_CTX_create(1920, 1080, SZ_IMAGETYPE_BGR);
  if (imgCtx == NULL) goto JUMP;

  //创建一个人脸检索数据库句柄
  databaseCtx = SZ_DATABASE_CTX_create("database");
  if (databaseCtx == NULL) goto JUMP;

  //输入一张jpg人脸照片,然后得到人脸特征值
  SZ_INT32 featureLen = 0;
  bOk = getFeature(jpgFile1, faceCtx, imgCtx, &pFeature, &featureLen);
  if (!bOk) goto JUMP;

  //增加张三人脸特征到database中
  UserInfo userInfo;
  memset(&userInfo, 0x00, sizeof(userInfo));
  userInfo.age = 29;
  userInfo.sex = 0;
  memcpy(userInfo.name, "zhangsan", 9);
  ret = SZ_DATABASE_add(databaseCtx, 0, pFeature, &userInfo,
                       sizeof(UserInfo));
  if (ret != SZ_RETCODE_OK) goto JUMP;

  //输入一张jpg人脸照片,然后得到人脸特征值
  bOk = getFeature(jpgFile2, faceCtx, imgCtx, &pFeature, &featureLen);
  if (!bOk) goto JUMP;

  //增加李四人脸特征到database中
  memset(&userInfo, 0x00, sizeof(userInfo));
  userInfo.age = 23;
  userInfo.sex = 1;
  memcpy(userInfo.name, "lisi", 5);
  ret = SZ_DATABASE_add(databaseCtx, 1, pFeature, &userInfo, sizeof(UserInfo));
  if (ret != SZ_RETCODE_OK) goto JUMP;

  ret = SZ_DATABASE_save(databaseCtx);
  if (ret != SZ_RETCODE_OK) goto JUMP;

  // ret = SZ_DATABASE_remove(databaseCtx, 1, &userInfo);
  if (ret != SZ_RETCODE_OK) goto JUMP;
  // printf("remove result = %s   %d   %d\n", userInfo.name, userInfo.age,
  // userInfo.sex);


  ret = SZ_DATABASE_save(databaseCtx);
  if (ret != SZ_RETCODE_OK) goto JUMP;

  //在database中检索张三人脸特征
  ret = SZ_DATABASE_query(databaseCtx, pFeature, &pQueryResults, &topN);
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
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_NET_CTX_release(netCtx);
  SZ_IMAGE_CTX_release(imgCtx);
  SZ_FACE_CTX_release(faceCtx);
  SZ_DATABASE_CTX_release(databaseCtx);
  return ret;
}
