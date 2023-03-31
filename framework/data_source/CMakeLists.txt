cmake_minimum_required(VERSION 3.22)
project(SnDatasource)
set(CMAKE_CXX_STANDARD 20)

add_library(SnDatasource STATIC "")

target_include_directories(SnDatasource PUBLIC ${COMMON_INC_DIR})

target_sources(SnDatasource PUBLIC
        IDataSource.cpp
        DataSourceFactory.cpp
        curl/CurlDataSource.cpp
        curl/CurlConnection.cpp
        curl/CurlConnectionManager.cpp
    )



