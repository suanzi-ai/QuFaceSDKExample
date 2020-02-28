/**
 * @file sdk_common.h
 * @brief
 *
 */

#ifndef __SDK_COMMON__H__
#define __SDK_COMMON__H__

#include "sz_common.h"
#include "sz_face_module.h"

#define MAX_FILE_LEN 256

SZ_BOOL getFeature(const char *jpgFile, SZ_HANDLE faceHandle,
                   SZ_HANDLE imgHandle, SZ_FACE_FEATURE **pFeature,
                   SZ_INT32 *pFeatureLen);

SZ_BOOL getImageFromjpg(const char *jpgFile, SZ_HANDLE *imgHandle);

#endif
