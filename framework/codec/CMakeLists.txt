cmake_minimum_required(VERSION 3.22)
project(framework_codec)
set(CMAKE_CXX_STANDARD 20)

add_library(framework_codec STATIC "")

target_include_directories(framework_codec PUBLIC
        ${COMMON_INC_DIR}
        )

target_sources(framework_codec PRIVATE
        ICodec.cpp
        )
