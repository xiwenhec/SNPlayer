cmake_minimum_required(VERSION 3.22)
project(framework)
set(CMAKE_CXX_STANDARD 20)

set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

#这里的framework目录不能用 "."表示，因为这个变量要传递给子目录里的cmakelist
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