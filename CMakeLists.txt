if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "${CMAKE_SOURCE_DIR}/cmake/himix200.toolchain.cmake"
      CACHE STRING "")
endif()

cmake_minimum_required(VERSION 3.10)

project(
  QUFaceSDKExample
  VERSION 0.1.0
  DESCRIPTION "QUFace SDK example")

option(DOWNLOAD_DEPENDENCY "Download 3rd party dependencies from remote" ON)

# -- HiSiSDK --
if(NOT DEFINED HISI_SDK_PREFIX)
  set(HISI_SDK_PREFIX "${CMAKE_SOURCE_DIR}/deps/hisi_sdk")
endif()

message(STATUS "HISI_SDK_PREFIX to: ${HISI_SDK_PREFIX}")
list(APPEND CMAKE_FIND_ROOT_PATH ${HISI_SDK_PREFIX})
list(APPEND CMAKE_PREFIX_PATH ${HISI_SDK_PREFIX})

# -- OpenCV --
if(NOT DEFINED OPENCV_PREFIX)
  set(OPENCV_PREFIX "${CMAKE_SOURCE_DIR}/deps/opencv")
endif()

message(STATUS "OPENCV_PREFIX to: ${OPENCV_PREFIX}")
list(APPEND CMAKE_FIND_ROOT_PATH ${OPENCV_PREFIX})
list(APPEND CMAKE_PREFIX_PATH ${OPENCV_PREFIX})

# -- QuFaceSDK --
if(NOT DEFINED QUFACE_SDK_PREFIX)
  set(QUFACE_SDK_PREFIX "${CMAKE_SOURCE_DIR}/deps/qufacesdk")
endif()

message(STATUS "QUFACE_SDK_PREFIX to: ${QUFACE_SDK_PREFIX}")
list(APPEND CMAKE_FIND_ROOT_PATH ${QUFACE_SDK_PREFIX})
list(APPEND CMAKE_PREFIX_PATH ${QUFACE_SDK_PREFIX})

if(DOWNLOAD_DEPENDENCY)
  # -- Download dependecies --
  include(cmake/download.cmake)

  set(DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/deps/download")

  download_and_extract(
    https://cdn.quvision.com/qufacesdk/qufacesdk-dep-opencv-0.1.0.tar.gz
    ${DOWNLOAD_DIR}/opencv.tgz
    SHA256
    aceefda0cb224ddb4cd4d61b78ff1eed80c54b4d39db1038d7b30d78f0ef4df9
    ${OPENCV_PREFIX})

  download_and_extract(
    https://cdn.quvision.com/qufacesdk/qufacesdk-dep-hisisdk-0.1.0.tar.gz
    ${DOWNLOAD_DIR}/hisisdk.tgz
    SHA256
    cb732f7c5908ba06c9c1c460ad6059e6fd11bf5e5caa7da0c105adad7adfc2bc
    ${HISI_SDK_PREFIX})

  download_and_extract(
    https://cdn.quvision.com/qufacesdk/QUFaceSDK-0.1.0-Linux.tar.gz
    ${DOWNLOAD_DIR}/qufacesdk.tgz
    SHA256
    eae29ccfbd48c775d636c8717db1eb53000d16d9f7243b5f2c4274769da0e168
    ${QUFACE_SDK_PREFIX})
endif()

find_package(QuFaceSDK REQUIRED)

# -- Project --
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB EXAMPLE_COMMON_FILES common/*.c*)
include_directories(common)

add_library(example_common STATIC ${EXAMPLE_COMMON_FILES})
target_link_libraries(example_common QuFaceSDK::common)

add_executable(face_compare face_compare.c)

target_link_libraries(
  face_compare
  example_common
  QuFaceSDK::face
  QuFaceSDK::image
  QuFaceSDK::common
  ${OpenCV_LIBS}
  ${HiSiSDK_LIBS})

add_executable(face_compare_multi face_compare_multi.c)

target_link_libraries(
  face_compare_multi
  example_common
  QuFaceSDK::face
  QuFaceSDK::image
  QuFaceSDK::common
  ${OpenCV_LIBS}
  ${HiSiSDK_LIBS})

add_executable(face_query face_query.c)

target_link_libraries(
  face_query
  example_common
  QuFaceSDK::face
  QuFaceSDK::database
  QuFaceSDK::image
  QuFaceSDK::common
  ${OpenCV_LIBS}
  ${HiSiSDK_LIBS})

install(TARGETS face_compare face_compare_multi face_query DESTINATION bin)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION bin)
