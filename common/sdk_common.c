#include "sdk_common.h"

#include "image.h"
#include "sz_face_module.h"
#include "sz_image_module.h"

SZ_BOOL getFeature(const char *jpgFile, SZ_HANDLE faceHandle,
                   SZ_HANDLE imgHandle, SZ_FACE_FEATURE **pFeature,
                   SZ_INT32 *pFeatureLen) {
  int width = 0, height = 0;
  unsigned char *pBgrData = jpg2Bgr(jpgFile, &width, &height);
  if (pBgrData == NULL) return SZ_FALSE;

  //更新图像句柄尺寸
  int faceCnt = 0;
  SZ_BOOL bOk = SZ_FALSE;
  SZ_RETCODE ret = gszImage.setData(imgHandle, pBgrData, width, height);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }

#if 1

  ret = gszFace.detect(faceHandle, imgHandle, &faceCnt);
  if (ret != SZ_RETCODE_OK || faceCnt <= 0) {
    goto JUMP;
  }

  SZ_FACE_QUALITY quality;
  ret = gszFace.evaluate(faceHandle, imgHandle, 0, &quality);
  printf("[INFO] getFeature(): face quality(%f, %f, %f, %f, %f, %f)\n",
         quality.pitch, quality.yaw, quality.roll, quality.leftScore,
         quality.rightScore, quality.mouthScore);

  //始终获取索引为0的脸的特征
  ret = gszFace.extractFeatureByIndex(faceHandle, imgHandle, 0, pFeature,
                                      pFeatureLen);
#else
  SZ_RECT rect = {0, 0, width, height};
  printf("%d %d %d %d \n", rect.x, rect.y, rect.width, rect.height);
  ret = gszFace.extractFeatureByPosition(faceHandle, imgHandle, &rect, pFeature,
                                         pFeatureLen);
#endif
  if (ret != SZ_RETCODE_OK) goto JUMP;
  bOk = SZ_TRUE;
JUMP:
  free(pBgrData);
  return bOk;
}

SZ_BOOL getImageFromjpg(const char *jpgFile, SZ_HANDLE *imgHandle) {
  int width = 0, height = 0;
  SZ_RETCODE ret;
  SZ_BOOL bOk = SZ_FALSE;

  unsigned char *pBgrData = jpg2Bgr(jpgFile, &width, &height);
  if (pBgrData == NULL) {
    printf("[ERR] getImageFromjpg: read jpgFile %s failed!\n", jpgFile);
    return SZ_FALSE;
  }

  if (*imgHandle == NULL) {
    ret = gszImage.createHandle(width, height, SZ_IMAGETYPE_BGR, imgHandle);
    if (ret != SZ_RETCODE_OK) {
      printf("[ERR] getImageFromjpg: creat imageHandle failed!\n");
      goto JUMP;
    }
  }

  ret = gszImage.setData(*imgHandle, pBgrData, width, height);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] getImageFromjpg: setData failed!\n");
    goto JUMP;
  }

  bOk = SZ_TRUE;
JUMP:
  free(pBgrData);
  return bOk;
}
