#include "image.h"

#include "imageutil.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *jpg2Bgr(const char *jpgFile, int *pWidth, int *pHeight) {
  cv::Mat img = cv::imread(jpgFile);
  if (img.empty()) {
    printf("%s is not exist!\n", jpgFile);
    return NULL;
  }

  int width = img.cols;
  int height = img.rows;

  //返回图像宽度和高度
  *pWidth = width;
  *pHeight = height;

  //分配存放BGR图像数据缓冲
  unsigned char *pData = (unsigned char *)malloc(width * height * 3);
  memcpy(pData, img.ptr(), width * height * 3);

  //返回BGR图像数据缓冲
  return pData;
}

unsigned char *jpg2Nv21(const char *jpgFile, int *pWidth, int *pHeight) {
  cv::Mat img = cv::imread(jpgFile);
  if (img.empty()) {
    printf("%s is not exist!\n", jpgFile);
    return NULL;
  }

  int width = img.cols;
  int height = img.rows;

  //分配存放NV21图像数据缓冲
  unsigned char *pData = (unsigned char *)malloc(width * height * 3 / 2);

  cv::Mat yuv420pImg;
  // bgr 转 yuv420p
  cvtColor(img, yuv420pImg, cv::COLOR_BGR2YUV_I420);

  //返回图像宽度和高度
  *pWidth = width;
  *pHeight = height;

  // yuv420p 转 nv21
  int len = width * height;
  memcpy(pData, yuv420pImg.ptr(), len);
  unsigned char *p = pData + len;
  unsigned char *u = yuv420pImg.ptr() + len;
  unsigned char *v = yuv420pImg.ptr() + len + len / 4;
  for (int i = 0; i < len / 4; i++) {
    p[i * 2] = v[i];
    p[i * 2 + 1] = u[i];
  }

  //返回NV21图像缓冲数据
  return pData;
}

#ifdef __cplusplus
}
#endif
