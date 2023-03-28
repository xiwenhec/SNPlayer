cmake_minimum_required(VERSION 3.22)
project(SnMedia)

set(CMAKE_CXX_STANDARD 20)


set(FRAMEWORK_DIR ${PROJECT_SOURCE_DIR}/../framework)

set(COMMON_INC_DIR ${FRAMEWORK_DIR})
add_subdirectory(${FRAMEWORK_DIR} framework.out)

add_library(SnMedia STATIC "")

target_include_directories(SnMedia PRIVATE  ${COMMON_INC_DIR})

target_sources(SnMedia PRIVATE
        SnPlayer.cpp
        MediaPlayer.cpp
        BufferController.cpp
        DeviceManager.cpp
        MediaPacketQueue.cpp
        PlayerMsgController.cpp
        PlayerMsgProcessor.cpp
        PlayerNotifier.cpp
        PlayerParams.cpp
        PlayerStatistic.cpp
        ReferClock.cpp
        )

target_link_libraries(SnMedia PRIVATE
        SnDecoder
        SnDemuxer
        SnUtils
        SnDatasource
        )