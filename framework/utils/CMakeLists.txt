cmake_minimum_required(VERSION 3.22)
project(framework_utils)
set(CMAKE_CXX_STANDARD 20)

add_library(framework_utils STATIC "")

target_sources(
        framework_utils PUBLIC
        SNJson.h
        NSLog.cpp
        SNThread.cpp
        SNTimer.cpp
        SNUtils.cpp
        SNFFUtils.cpp
)

target_sources(
        framework_utils PUBLIC
        ../base/RingBuffer.cpp
        ../base/media/ISNPacket.cpp
        ../base/media/IAVBSF.cpp
        ../base/media/SNAVPacket.cpp
)

target_include_directories(framework_utils PUBLIC
        ${COMMON_INC_DIR}
        )