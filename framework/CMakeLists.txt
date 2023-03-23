cmake_minimum_required(VERSION 3.22)
project(framework)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -fstandalone-debug")

add_subdirectory(utils)
add_subdirectory(data_source)
add_subdirectory(demuxer)
add_subdirectory(codec)