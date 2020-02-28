#include "imageutil.h"

#include <algorithm>
/**
        双线性插值
***/
bool resize(Image *pDest, const Image *pSrc) {
  if (pSrc == NULL || pDest == NULL || pSrc->type != pDest->type) return false;

  if (pSrc->type == SZ_IMAGETYPE_BGR || pSrc->type == SZ_IMAGETYPE_RGB ||
      pSrc->type == SZ_IMAGETYPE_GRAY)
    ;
  else
    return false;

  int chNum = 3;
  if (pSrc->type == SZ_IMAGETYPE_GRAY) {
    chNum = 1;
  }

  float scaleX = (float)pSrc->width / pDest->width;
  float scaleY = (float)pSrc->height / pDest->height;
  int sstep = pSrc->width * chNum;
  int dstep = pDest->width * chNum;
  for (int i = 0; i < pDest->height; i++) {
    unsigned char *pDstData = pDest->pData;
    float v = (float)((i + 0.5) * scaleY - 0.5);
    int y = (int)v;  //整数部分
    v -= y;          //小数部分
    if (y < 0) {
      v = 0;
      y = 0;
    }
    if (y > pSrc->height - 2) {
      y = pSrc->height - 2;
      v = 1;
    }

    short cbufy[2];
    cbufy[0] = (short)((1.f - v) * 2048);
    cbufy[1] = 2048 - cbufy[0];
    unsigned char *pSrcData = pSrc->pData;
    for (int j = 0; j < pDest->width; j++) {
      float u = (float)((j + 0.5) * scaleX - 0.5);
      int x = (int)u;
      u -= x;
      if (x < 0) {
        x = 0;
        u = 0;
      }
      if (x > pSrc->width - 2) {
        x = pSrc->width - 2;
        u = 1;
      }
      short cbufx[2];
      cbufx[0] = (short)((1.f - u) * 2048);
      cbufx[1] = 2048 - cbufx[0];
      for (int k = 0; k < chNum; k++) {
        *(pDstData + i * dstep + chNum * j + k) =
            (*(pSrcData + y * sstep + chNum * x + k) * cbufx[0] * cbufy[0] +
             *(pSrcData + (y + 1) * sstep + chNum * x + k) * cbufx[0] *
                 cbufy[1] +
             *(pSrcData + y * sstep + chNum * (x + 1) + k) * cbufx[1] *
                 cbufy[0] +
             *(pSrcData + (y + 1) * sstep + chNum * (x + 1) + k) * cbufx[1] *
                 cbufy[1]) >>
            22;
      }
    }
  }
  return true;
}

bool bgrToRgb(Image *pDest, const Image *pSrc) {
  if (pDest->width != pSrc->width || pDest->height != pSrc->height ||
      pDest->dataLen != pSrc->dataLen || pSrc->type != SZ_IMAGETYPE_BGR)
    return false;

  int w = pSrc->width;
  int h = pSrc->height;
  int step = w * 3;
  const unsigned char *pSrcData = pSrc->pData;
  unsigned char *pDestData = pDest->pData;
  if (pSrcData == pDestData) {
    for (int i = 0; i < h; i++) {
      //获取第i行首像素指针
      unsigned char *pRow = &pDestData[i * step];
      for (int j = 0; j < step; j += 3) {
        //将img的bgr转为image的rgb
        unsigned char tmp = pRow[j + 2];
        pRow[j + 2] = pRow[j];
        pRow[j] = tmp;
      }
    }
  } else {
    for (int i = 0; i < h; i++) {
      //获取第i行首像素指针
      const unsigned char *pSrcRow = &pSrcData[i * step];
      unsigned char *pDestRow = &pDestData[i * step];
      for (int j = 0; j < step; j += 3) {
        //将img的bgr转为image的rgb
        pDestRow[j + 2] = pSrcRow[j];
        pDestRow[j + 1] = pSrcRow[j + 1];
        pDestRow[j] = pSrcRow[j + 2];
      }
    }
  }
  return true;
}

bool transpose(Image *pDest, const Image *pSrc) {
  if (pDest->width != pSrc->height || pDest->height != pSrc->width ||
      pDest->dataLen != pSrc->dataLen || pSrc->type != pDest->type)
    return false;

  if (pSrc->type == SZ_IMAGETYPE_BGR || pSrc->type == SZ_IMAGETYPE_RGB ||
      pSrc->type == SZ_IMAGETYPE_GRAY)
    ;
  else
    return false;

  int chNum = 3;
  if (pSrc->type == SZ_IMAGETYPE_GRAY) {
    chNum = 1;
  }
  if (pDest->pData == pSrc->pData) {
    if (pSrc->width != pSrc->height) return false;
    int n = pDest->height;
    int step = n * chNum;
    unsigned char *pDestData = pDest->pData;
    for (int i = 0; i < n; i++) {
      unsigned char *pRow = (unsigned char *)(pDestData + step * i);
      int i_ = i * chNum;
      for (int j = i + 1; j < n; j++) {
        unsigned char *pData1 = (unsigned char *)(pDestData + step * j);
        int j_ = j * chNum;
        for (int ch = 0; ch < chNum; ch++) {
          std::swap(pRow[j_ + ch], pData1[i_ + ch]);
        }
      }
    }
  } else {
    const unsigned char *pSrcData = pSrc->pData;
    unsigned char *pDestData = pDest->pData;
    int m = pSrc->width, n = pSrc->height;
    int dstep = pDest->width * chNum;
    int sstep = m * chNum;
    for (int i = 0; i < n; i++) {
      const unsigned char *s = (const unsigned char *)(pSrcData + i * sstep);
      int i_ = i * chNum;
      for (int j = 0; j < m; j++) {
        unsigned char *d = (unsigned char *)(pDestData + j * dstep);
        int j_ = j * chNum;
        for (int ch = 0; ch < chNum; ch++) {
          d[i_ + ch] = s[j_ + ch];
        }
      }
    }
  }
  return true;
}

bool bgrPlanar2BgrPacked(Image *pDest, const Image *pSrc) {
  if (pDest->width != pSrc->height || pDest->height != pSrc->width ||
      pDest->dataLen != pSrc->dataLen || pSrc->type != pDest->type ||
      pSrc->type != SZ_IMAGETYPE_BGR)
    return false;
  int len = pSrc->width * pSrc->height;
  const unsigned char *pb = pSrc->pData;
  const unsigned char *pg = pb + len;
  const unsigned char *pr = pg + len;
  unsigned char *pDst = pDest->pData;
  for (int i = 0; i < len; i++) {
    pDst[i * 3] = pb[i];
    pDst[i * 3 + 1] = pg[i];
    pDst[i * 3 + 2] = pr[i];
  }
}

bool bgrPacked2BgrPlanar(Image *pDest, const Image *pSrcImage) {
  if (pDest->width != pSrcImage->height || pDest->height != pSrcImage->width ||
      pDest->dataLen != pSrcImage->dataLen || pSrcImage->type != pDest->type ||
      pSrcImage->type != SZ_IMAGETYPE_BGR)
    return false;
  int len = pSrcImage->width * pSrcImage->height;
  unsigned char *pb = pDest->pData;
  unsigned char *pg = pb + len;
  unsigned char *pr = pg + len;
  const unsigned char *pSrc = pSrcImage->pData;
  for (int i = 0; i < len; i++) {
    pb[i] = pSrc[i * 3];
    pg[i] = pSrc[i * 3 + 1];
    pr[i] = pSrc[i * 3 + 2];
  }
}

void bgrPlanar2BgrPacked(char *pDst, const char *pSrc, int width, int height) {
  int len = width * height;
  const char *pb = pSrc;
  const char *pg = pb + len;
  const char *pr = pg + len;
  for (int i = 0; i < len; i++) {
    pDst[i * 3] = pb[i];
    pDst[i * 3 + 1] = pg[i];
    pDst[i * 3 + 2] = pr[i];
  }
}

void bgrPacked2BgrPlanar(char *pDst, const char *pSrc, int width, int height) {
  int len = width * height;
  char *pb = pDst;
  char *pg = pb + len;
  char *pr = pg + len;
  for (int i = 0; i < len; i++) {
    pb[i] = pSrc[i * 3];
    pg[i] = pSrc[i * 3 + 1];
    pr[i] = pSrc[i * 3 + 2];
  }
}

bool crop(Image *pDst, const Image *pSrc, const Rect *pRect) {
  if (pDst->width != pRect->width || pDst->height != pRect->height ||
      pSrc->type != pDst->type || pSrc->width < pRect->width + pRect->x ||
      pSrc->height < pRect->height + pRect->y) {
    return false;
  }

  if (pSrc->type == SZ_IMAGETYPE_BGR || pSrc->type == SZ_IMAGETYPE_RGB ||
      pSrc->type == SZ_IMAGETYPE_GRAY)
    ;
  else
    return false;

  int chNum = 3;
  if (pSrc->type == SZ_IMAGETYPE_GRAY) {
    chNum = 1;
  }

  unsigned char *pDstData = pDst->pData;
  const unsigned char *pSrcData = pSrc->pData;
  int sstrid = pSrc->width * chNum;
  int dstrid = pDst->width * chNum;
  for (int i = 0; i < pDst->height; i++) {
    for (int j = 0; j < dstrid; j++) {
      pDstData[i * dstrid + j] =
          pSrcData[(i + pRect->y) * sstrid + pRect->x * chNum + j];
    }
  }
  return true;
}
