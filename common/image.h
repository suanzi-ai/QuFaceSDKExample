/**
 * @file image.h
 * @brief 基本图像定义
 *
 */

#ifndef __IMAGE__H__
#define __IMAGE__H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *jpg2Bgr(const char *jpgFile, int *pWidth, int *pHeight);

unsigned char *jpg2Nv21(const char *jpgFile, int *pWidth, int *pHeight);

#ifdef __cplusplus
}
#endif

#endif
