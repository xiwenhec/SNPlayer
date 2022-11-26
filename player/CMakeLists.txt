cmake_minimum_required(VERSION 3.22)
project(SivinPlayer)

set(CMAKE_CXX_STANDARD 20)


set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

add_subdirectory(${TOPDIR}/framework framework.out)

add_executable(SivinPlayer SivinPlayer.cpp)