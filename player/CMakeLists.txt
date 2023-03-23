cmake_minimum_required(VERSION 3.22)
project(SivinPlayer)

set(CMAKE_CXX_STANDARD 20)


set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

set(COMMON_INC_DIR ${TOPDIR}/framework
        ${TOPDIR}/third_party/json
        ${TOPDIR}/third_party/curl/include
        ${TOPDIR}/third_party/openssl/include
        ${TOPDIR}/third_party/ffmpeg/x86_64/include
        )

set(COMMON_LIB_DIR ${TOPDIR}/third_party/curl/lib
        ${TOPDIR}/third_party/openssl/lib
        ${TOPDIR}/third_party/ffmpeg/x86_64/lib
        )


add_subdirectory(${TOPDIR}/framework framework.out)

add_library(SivinPlayer STATIC "")


target_sources(SivinPlayer PRIVATE
        SivinPlayer.cpp
        PlayerMsgProcessor.cpp
        PlayerMsgController.cpp
        PlayerParamsSet.cpp
        MediaPlayer.cpp
        )


target_link_libraries(SivinPlayer PRIVATE
        framework_codec
        framework_demuxer
        framework_utils
        framework_datasource
        pthread
        avformat
        avcodec
        avutil
        curl
        ssl
        crypto
        z
        )