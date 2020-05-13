cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "${PROJECT_SOURCE_DIR}/cmake/himix200.toolchain.cmake"
      CACHE STRING "")
endif()

set(CMAKE_GENERATOR "Ninja")
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Set up ccache ...")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

project(
  QuFaceSDKExample
  VERSION 0.1.0
  DESCRIPTION "QuFace SDK example")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

option(DOWNLOAD_DEPENDENCY "Download 3rd party dependencies from remote" ON)

set(PROJECT_DEPENDENCY_DIR
    "${PROJECT_SOURCE_DIR}/deps"
    CACHE STRING "Project dependencies dir")
set(RESOURCE_PREFIX
    "${PROJECT_SOURCE_DIR}/resources"
    CACHE STRING "Dir for model & font file")
set(HISI_SDK_PREFIX
    "${PROJECT_DEPENDENCY_DIR}/${HISI_SDK_PLATFORM}"
    CACHE STRING "Dir for hisi sdk")
set(THIRD_PARTY_PREFIX
    "${PROJECT_DEPENDENCY_DIR}/3rd"
    CACHE STRING "Dir for 3rd")
set(QUFACE_SDK_PREFIX
    "${PROJECT_DEPENDENCY_DIR}/qufacesdk"
    CACHE STRING "Dir for quface sdk")

message(STATUS "set HISI_SDK_PREFIX to: ${HISI_SDK_PREFIX}")
message(STATUS "set THIRD_PARTY_PREFIX to: ${THIRD_PARTY_PREFIX}")
message(STATUS "set QUFACE_SDK_PREFIX to: ${QUFACE_SDK_PREFIX}")

set(CMAKE_FIND_ROOT_PATH ${HISI_SDK_PREFIX} ${THIRD_PARTY_PREFIX}
                         ${QUFACE_SDK_PREFIX})
set(CMAKE_PREFIX_PATH ${HISI_SDK_PREFIX} ${THIRD_PARTY_PREFIX}
                      ${QUFACE_SDK_PREFIX})

if(DOWNLOAD_DEPENDENCY)
  # -- Download dependecies --
  include(cmake/download.cmake)

  set(DOWNLOAD_DIR "${PROJECT_DEPENDENCY_DIR}/download")

  download_and_extract(
    https://quvision.oss-cn-beijing.aliyuncs.com/qufacesdk/deps/prebuild-3rd-0.1.2.tar.gz
    ${DOWNLOAD_DIR}/prebuild-3rd-0.1.2.tar.gz
    SHA256
    2f129d529e599395b29b21b6d7d5b694b5f158f573433a2f911c834cb314e148
    ${THIRD_PARTY_PREFIX})

  download_and_extract(
    https://quvision.oss-cn-beijing.aliyuncs.com/qufacesdk/hisi/rp-dv300-sdk.tgz
    ${DOWNLOAD_DIR}/rp-dv300-sdk.tgz
    SHA256
    f9075af18adbb95a21789f6d79ca98fa1f92fee0696a2d156a88425fa4efdbcb
    ${HISI_SDK_PREFIX})

  download_and_extract(
    https://quvision.oss-cn-beijing.aliyuncs.com/qufacesdk/releases/QuFaceSDK-0.10.1-hisi-rp-dv300.tar.gz
    ${DOWNLOAD_DIR}/QuFaceSDK-0.10.1-hisi-rp-dv300.tar.gz
    SHA256
    ad0100acde0b0fef2c8715c6a30b117f2871864f6f10a9e26b10812635bab2ef
    ${QUFACE_SDK_PREFIX})

  download_and_extract(
    https://quvision.oss-cn-beijing.aliyuncs.com/qufacesdk/hisi/hisi-model-1.0.0.zip
    ${DOWNLOAD_DIR}/hisi-model-1.0.0.zip
    SHA256
    2718f5af7a2cfe7211734c18a432f73c3f537d02723184cf1ace6c072b340cea
    ${RESOURCE_PREFIX}/models)
endif()

find_package(QuFaceSDK REQUIRED)

file(GLOB EXAMPLE_COMMON_FILES common/*.c*)
include_directories(common)

add_library(example_common STATIC ${EXAMPLE_COMMON_FILES})
target_link_libraries(example_common QuFaceSDK::common)

add_executable(face_compare_multi face_compare_multi.c)
target_link_libraries(face_compare_multi example_common QuFaceSDK::face)

add_executable(face_compare face_compare.c)
target_link_libraries(face_compare example_common QuFaceSDK::face)

add_executable(face_db_server face_db_server.c)
target_link_libraries(face_db_server example_common QuFaceSDK::face
                      QuFaceSDK::database QuFaceSDK::faceServer)

add_executable(face_detect face_detect.c)
target_link_libraries(face_detect example_common QuFaceSDK::face)

add_executable(face_detect_mscale face_detect_mscale.c)
target_link_libraries(face_detect_mscale example_common QuFaceSDK::face)

add_executable(face_query face_query.c)
target_link_libraries(face_query example_common QuFaceSDK::face
                      QuFaceSDK::database)

install(TARGETS face_compare_multi face_compare face_db_server face_detect
                face_query DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data ${RESOURCE_PREFIX}/models/
        DESTINATION .)
