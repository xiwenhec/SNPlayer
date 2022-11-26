cmake_minimum_required(VERSION 3.22)
project(DataSourceTest)

set(CMAKE_CXX_STANDARD 20)


set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

add_subdirectory(${TOPDIR}/framework framework.out)

set(ENABLE_THREAD_TEST ON)

if (${ENABLE_THREAD_TEST})
    add_executable(Test thread/ThreadTest.cpp)
endif ()

