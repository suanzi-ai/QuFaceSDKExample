#include "model.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned char *loadModel(const char *modelFile, int *pModelLen) {
  FILE *fp = fopen(modelFile, "r");
  if (fp == NULL) return NULL;

  //获取模型文件长度
  fseek(fp, 0L, SEEK_END);
  int fileLen = 0;
  if ((fileLen = ftell(fp)) == -1) {
    fclose(fp);
    return NULL;
  }

  //分配模型缓冲
  unsigned char *pModelData = malloc(fileLen);
  if (pModelData == NULL) {
    fclose(fp);
    return NULL;
  }

  printf("[INFO] loadModel: model filelen = %d\n", fileLen);
  //读模型数据到模型缓冲pModelData
  rewind(fp);  // fseek(fp, 0L, SEEK_SET)的另一种写法
  *pModelLen = fread(pModelData, 1, fileLen, fp);
  if ((*pModelLen) != fileLen) {
    fclose(fp);
    return NULL;
  }

  fclose(fp);
  return pModelData;
}
