cmake_minimum_required(VERSION 3.22)
project(framework_utils)
set(CMAKE_CXX_STANDARD 20)

add_library(framework_utils STATIC "")

target_sources(
        framework_utils PRIVATE
        thread/SNThread.cpp
)