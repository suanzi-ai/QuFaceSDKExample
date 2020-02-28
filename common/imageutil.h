#ifndef _IMAGE_UTIL_H_
#define _IMAGE_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sz_common.h"

typedef struct Rect_ {
  int x;
  int y;
  int width;
  int height;

  bool init(int x, int y, int width, int height) {
    if (width > 3840 || width < 0 || height < 0 || height > 2160) return false;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    return true;
  }
} Rect;

typedef struct Image_ {
  int type;
  int width;
  int height;
  int dataLen;
  unsigned char *pData;

  explicit Image_(int width, int height, SZ_IMAGETYPE type) {
    int len = width * height;
    switch (type) {
      case SZ_IMAGETYPE_BGR:
      case SZ_IMAGETYPE_RGB:
        len *= 3;
        break;
      case SZ_IMAGETYPE_NV21:
        len *= 3 / 2;
        break;
      case SZ_IMAGETYPE_GRAY:
        break;
      default:
        break;
    }
    this->width = width;
    this->height = height;
    this->dataLen = len;
    this->type = type;
    this->pData = (unsigned char *)malloc(len);
  }

  bool setData(const unsigned char *pData, int width, int height) {
    int len = setSize(width, height);
    if (len == 0) return false;
    memcpy(this->pData, pData, len);
    return true;
  }

  int setSize(int width, int height) {
    int len = 0;
    switch (type) {
      case SZ_IMAGETYPE_BGR:
      case SZ_IMAGETYPE_RGB:
        len = width * height * 3;
        break;
      case SZ_IMAGETYPE_GRAY:
        len = width * height;
        break;
      case SZ_IMAGETYPE_NV21:
        len = width * height * 3 / 2;
        break;
      default:
        return 0;
        break;
    }
    if (len > dataLen) {
      pData = (unsigned char *)realloc(pData, len);
      dataLen = len;
    }
    this->width = width;
    this->height = height;
    return len;
  }

  ~Image_() {
    if (pData != NULL) {
      dataLen = 0;
      free(pData);
    }
  }
} Image;

bool resize(Image *pDest, const Image *pSrc);
bool bgrToRgb(Image *pDest, const Image *pSrc);
bool transpose(Image *pDest, const Image *pSrc);
bool bgrPlanar2BgrPacked(Image *pDest, const Image *pSrc);
bool bgrPacked2BgrPlanar(Image *pDest, const Image *pSrc);
void bgrPlanar2BgrPacked(char *pDst, const char *pSrc, int width, int height);
void bgrPacked2BgrPlanar(char *pDst, const char *pSrc, int width, int height);
bool crop(Image *pDst, const Image *pSrc, const Rect *pRect);

#endif
