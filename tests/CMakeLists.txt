cmake_minimum_required(VERSION 3.22)
project(DataSourceTest)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -Werror=return-type -Wuninitialized -fstandalone-debug")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-gc-sections")


set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

set(COMMON_INC_DIR  ${TOPDIR}/framework
        ${TOPDIR}/thirdparty/json
        ${TOPDIR}/thirdparty/curl/include
        ${TOPDIR}/thirdparty/openssl/include
        ${TOPDIR}/thirdparty/ffmpeg/x86_64/include
        )

set(COMMON_LIB_DIR ${TOPDIR}/thirdparty/curl/lib
        ${TOPDIR}/thirdparty/openssl/lib
        ${TOPDIR}/thirdparty/ffmpeg/x86_64/lib
        )

add_subdirectory(${TOPDIR}/framework framework.out)

add_executable(test "")


target_include_directories(test PUBLIC
        ${COMMON_INC_DIR}
        )

target_link_directories(test PRIVATE ${COMMON_LIB_DIR})

#线程测试
#set(ENABLE_THREAD_TEST ON)

#dataSource测试
#set(ENABLE_DATASOURCE_TEST ON)


set(ENABLE_DEMUXER_TEST ON)

if (${ENABLE_THREAD_TEST})
    target_sources(test PRIVATE
            thread/ThreadTest.cpp
            )
endif ()


if (${ENABLE_DATASOURCE_TEST})
    target_sources(test PRIVATE
            datasource/DataSourceTest.cpp
            )
endif ()

if (${ENABLE_DEMUXER_TEST})
    target_sources(test PRIVATE
            demuxer/DemuxerTest.cpp
            )
endif ()


target_link_libraries(test PRIVATE
        framework_codec
        framework_demuxer
        framework_utils
        framework_datasource
        pthread
        avformat
        avcodec
        avutil
        curl
        ssl
        crypto
        z
        )
