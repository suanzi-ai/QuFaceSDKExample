#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "image.h"
#include "sdk_common.h"
#include "sz_database_module.h"
#include "sz_face_module.h"
#include "sz_face_server_module.h"
#include "sz_image_module.h"
#include "sz_license_module.h"
#include "sz_net_module.h"

static void usage(char** argv) {
  printf(
      "usage: %s [a]  \n"
      "       facemodel\n",
      argv[0]);
  printf(
      "for example:\n"
      "%s facemodel\n",
      argv[0]);
}

int main(int argc, char** argv) {
  char modelFile[MAX_FILE_LEN + 1] = {0};

  //解析参数
  if (argc > 1) {
    strncpy(modelFile, argv[1], MAX_FILE_LEN);
    printf("[INFO] input model path:  %s \n", modelFile);
  } else {
    usage(argv);
    return -1;
  }

  SZ_RETCODE ret;
  SZ_NET_CTX* netCtx = NULL;
  SZ_LICENSE_CTX* licenseCtx = NULL;
  SZ_FACE_CTX* faceCtx = NULL;
  ret = init_handles(modelFile, &faceCtx, &licenseCtx, &netCtx);
  if (ret != SZ_RETCODE_OK) {
    goto JUMP;
  }

  SZ_DATABASE_CTX* databaseCtx = NULL;
  databaseCtx = SZ_DATABASE_CTX_create("database");
  if (databaseCtx == NULL) {
    goto JUMP;
  }

  SZ_FACE_SERVER_CTX* faceServerCtx = NULL;
  faceServerCtx = SZ_FACE_SERVER_CTX_create(netCtx, faceCtx, databaseCtx);
  if (faceServerCtx == NULL) {
    goto JUMP;
  }

  SZ_NET_join(netCtx);

JUMP:
  SZ_LICENSE_CTX_release(licenseCtx);
  SZ_NET_CTX_release(netCtx);
  SZ_FACE_CTX_release(faceCtx);
  SZ_DATABASE_CTX_release(databaseCtx);
  SZ_FACE_SERVER_CTX_release(faceServerCtx);
  return ret;
}
