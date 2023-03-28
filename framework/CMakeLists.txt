cmake_minimum_required(VERSION 3.22)
project(framework)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -fstandalone-debug")

set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

set(COMMON_INC_DIR 
        ${TOPDIR}/framework
        ${TOPDIR}/third_party/json
        ${TOPDIR}/third_party/curl/include
        ${TOPDIR}/third_party/openssl/include
        ${TOPDIR}/third_party/ffmpeg/x86_64/include
        )

add_subdirectory(utils)
add_subdirectory(data_source)
add_subdirectory(demuxer)
add_subdirectory(codec)