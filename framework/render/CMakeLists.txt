cmake_minimum_required(VERSION 3.22)
project(SnRender)
set(CMAKE_CXX_STANDARD 20)

add_library(SnRender STATIC "")

target_include_directories(SnDemuxer PUBLIC
        ${COMMON_INC_DIR}
        )

target_sources(SnRender PUBLIC
        )



