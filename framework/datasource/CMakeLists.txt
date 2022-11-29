cmake_minimum_required(VERSION 3.22)
project(framework_datasource)
set(CMAKE_CXX_STANDARD 20)

add_library(framework_datasource STATIC "")

set(ENABLE_CURL ON)


target_include_directories(framework_datasource PUBLIC
        ${COMMON_INC_DIR}
        )

if (${ENABLE_CURL})
    target_sources(framework_datasource PRIVATE
            IDataSource.cpp
            curl/CurlDataSource.cpp
            curl/CurlConnection.cpp
            )
endif ()


