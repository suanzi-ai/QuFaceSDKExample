#include "sdk_common.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <nlohmann/json.hpp>

#include "opencv2/opencv.hpp"
#include "sz_head_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

void saveDetection2File_head(const char *jpgFile, const char *pData, int w,
                             int h, int iModel,
                             SZ_HEAD_DETECTION_LIST detectResult) {
  SZ_RETCODE ret;
  cv::Mat image(h, w, CV_8UC3, (void *)pData);
  for (SZ_INT32 idx = 0; idx < detectResult.count; idx++) {
    auto head = detectResult.heads[idx];
    printf("model-%d: Head-%d [x=%d, y=%d, w=%d, h=%d]\n", iModel, idx,
           head.rect.x, head.rect.y, head.rect.width, head.rect.height);

    cv::rectangle(
        image,
        cv::Rect(head.rect.x, head.rect.y, head.rect.width, head.rect.height),
        cv::Scalar(255, 0, 0), 2);
  }

  std::string saveName =
      std::string(jpgFile) + "_scale_" + std::to_string(iModel) + ".jpg";
  cv::imwrite(saveName.c_str(), image);
}

SZ_RETCODE init_handles_head(const char *modelFile, SZ_HEAD_CTX **pHeadCtx,
                             SZ_LICENSE_CTX **pLicenseCtx,
                             SZ_HEAD_CONFIG *config) {
  SZ_NET_CTX *netCtx = NULL;
  if (init_handles_ex_head(modelFile, pHeadCtx, pLicenseCtx, &netCtx, config) !=
      SZ_RETCODE_OK) {
    return SZ_RETCODE_FAILED;
  }
  SZ_NET_detach(netCtx);
  SZ_NET_CTX_release(netCtx);
  return SZ_RETCODE_OK;
}

SZ_RETCODE init_handles_ex_head(const char *modelFile, SZ_HEAD_CTX **pHeadCtx,
                                SZ_LICENSE_CTX **pLicenseCtx,
                                SZ_NET_CTX **pNetCtx, SZ_HEAD_CONFIG *config) {
  SZ_RETCODE ret;

  ret = SZ_CONFIG_setFromEnv();
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] load config from env failed\n");
  }

  NetCreateOption opts = {0};
  opts.storagePath = (char *)".";
  opts.clientId = (char *)"QufaceHisiDemo";

  std::ifstream i("device_info.json");
  if (!i.is_open()) {
    printf("[ERR] device_info.json not present\n");
    return SZ_RETCODE_FAILED;
  }

  nlohmann::json device_info;
  i >> device_info;

  if (!device_info.contains("ProductKey") ||
      !device_info.contains("DeviceName") ||
      !device_info.contains("DeviceSecret") ||
      device_info["ProductKey"].empty() || device_info["DeviceName"].empty() ||
      device_info["DeviceSecret"].empty()) {
    printf(
        "[ERR] field ProductKey or DeviceName or DeviceSecret is missing in "
        "%s\n",
        "device_info.json");
    return SZ_RETCODE_FAILED;
  }
  std::string productKey = device_info["ProductKey"];
  std::string deviceName = device_info["DeviceName"];
  std::string deviceSecret = device_info["DeviceSecret"];
  opts.productKey = (char *)productKey.c_str();
  opts.deviceName = (char *)deviceName.c_str();
  opts.deviceSecret = (char *)deviceSecret.c_str();

  *pNetCtx = SZ_NET_CTX_create(opts);
  if (*pNetCtx == NULL) {
    printf("[ERR] SZ_NET_CTX_create failed !\n");
    return SZ_RETCODE_FAILED;
  }

  ret = SZ_NET_connect(*pNetCtx);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] SZ_NET_connect failed !\n");
    return ret;
  }

  *pLicenseCtx = SZ_LICENSE_CTX_create(*pNetCtx);
  if (*pLicenseCtx == NULL) {
    printf("[ERR] SZ_LICENSE_CTX_create failed !\n");
    return SZ_RETCODE_FAILED;
  }

  ret = SZ_LICENSE_auth(*pLicenseCtx);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] SZ_LICENSE_auth failed !\n");
    return ret;
  }

  *pHeadCtx = SZ_HEAD_CTX_create(*pLicenseCtx, config);
  if (*pHeadCtx == NULL) {
    printf("[ERR] SZ_HEAD_CTX_create failed !\n");
    return SZ_RETCODE_FAILED;
  }

  ret = SZ_HEAD_loadModelFromFile(*pHeadCtx, modelFile);
  if (ret != SZ_RETCODE_OK) {
    printf("[ERR] SZ_HEAD_loadModelFromFile failed !\n");
    return ret;
  }

  return SZ_RETCODE_OK;
}
