cmake_minimum_required(VERSION 3.22)
project(SnDecoder)
set(CMAKE_CXX_STANDARD 20)

add_library(SnDecoder STATIC "")

target_include_directories(SnDecoder PUBLIC
        ${COMMON_INC_DIR}
        )

target_sources(SnDecoder PRIVATE
        IDecoder.cpp
        )
