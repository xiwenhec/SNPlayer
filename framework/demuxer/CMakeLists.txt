cmake_minimum_required(VERSION 3.22)
project(SnDemuxer)
set(CMAKE_CXX_STANDARD 20)

add_library(SnDemuxer STATIC "")

target_include_directories(SnDemuxer PRIVATE
        ${COMMON_INC_DIR}
        )

target_sources(SnDemuxer PRIVATE
        IDemuxer.cpp
        AVFFDemuxer.cpp
        DemuxerService.cpp
        DemuxerFactory.cpp
        )



