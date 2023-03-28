cmake_minimum_required(VERSION 3.22)
project(SnUtils)
set(CMAKE_CXX_STANDARD 20)

add_library(SnUtils STATIC "")


target_include_directories(SnUtils PRIVATE
        ${COMMON_INC_DIR}
        )

target_sources(
        SnUtils PRIVATE
        SNJson.h
        SNLog.cpp
        SNThread.cpp
        SNTimer.cpp
        SNUtils.cpp
        SNFFUtil.cpp
        os/SNSysInfoUtil.cpp
)

target_sources(
        SnUtils PRIVATE
        ../base/RingBuffer.cpp
        ../base/media/SNPacket.cpp
        ../base/media/IAVBSF.cpp
        ../base/media/SNAVPacket.cpp
)

