cmake_minimum_required(VERSION 3.22)
project(framework_demuxer)
set(CMAKE_CXX_STANDARD 20)

add_library(framework_demuxer STATIC "")

target_include_directories(framework_demuxer PUBLIC
        ${COMMON_INC_DIR}
        )

target_sources(framework_demuxer PRIVATE
        IDemuxer.cpp
        AVFFDemuxer.cpp
        DemuxerService.cpp
        DemuxerFactory.cpp
        )



