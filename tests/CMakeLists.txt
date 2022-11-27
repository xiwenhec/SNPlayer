cmake_minimum_required(VERSION 3.22)
project(DataSourceTest)

set(CMAKE_CXX_STANDARD 20)


set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

set(COMMON_INC_DIR ${TOPDIR}/thirdparty/json ${TOPDIR}/framework)

add_subdirectory(${TOPDIR}/framework framework.out)

set(ENABLE_THREAD_TEST ON)

if (${ENABLE_THREAD_TEST})
    add_executable(Test "")

    target_include_directories(Test PUBLIC
            ${COMMON_INC_DIR}
            )

    target_sources(Test PRIVATE
            thread/ThreadTest.cpp
            )

    target_link_libraries(Test PRIVATE
            pthread
            framework_utils
            )
endif ()

