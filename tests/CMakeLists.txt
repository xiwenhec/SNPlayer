cmake_minimum_required(VERSION 3.22)
project(DataSourceTest)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -Werror=return-type -Wuninitialized -fstandalone-debug")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-gc-sections")


set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

set(COMMON_INC_DIR  
        ${TOPDIR}/media_player
        ${TOPDIR}/framework
        ${TOPDIR}/third_party/json
        ${TOPDIR}/third_party/curl/include
        ${TOPDIR}/third_party/openssl/include
        ${TOPDIR}/third_party/ffmpeg/x86_64/include
        )

set(COMMON_LIB_DIR 
        ${TOPDIR}/thirdparty/curl/lib
        ${TOPDIR}/third_party/openssl/lib
        ${TOPDIR}/third_party/ffmpeg/x86_64/lib
        )

add_subdirectory(${TOPDIR}/framework framework.out)
add_subdirectory(${TOPDIR}/media_player media_player.out)


add_executable(test "")

target_include_directories(test PRIVATE
        ${COMMON_INC_DIR}
        )
target_link_directories(test PRIVATE ${COMMON_LIB_DIR})





#线程测试
#set(ENABLE_THREAD_TEST ON)
#dataSource测试
#set(ENABLE_DATASOURCE_TEST ON)
set(ENABLE_DEMUXER_TEST ON)
set(ENABLE_PLAYER_TEST ON)


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
                SnDecoder
                SnDemuxer
                SnUtils
                SnDatasource
                pthread
                avformat
                avcodec
                avutil
                curl
                ssl
                crypto
                z
        )


if (${ENABLE_PLAYER_TEST}) 
        add_executable(PlayerTest "")

        target_sources(PlayerTest PRIVATE 
                player/PlayerTest.cpp
        )

        target_include_directories(PlayerTest PUBLIC
                ${COMMON_INC_DIR}
        )

        target_link_directories(PlayerTest PRIVATE ${COMMON_LIB_DIR})
        

        target_link_libraries(PlayerTest PRIVATE
                SnDecoder
                SnDemuxer
                SnUtils
                SnDatasource
                pthread
                avformat
                avcodec
                avutil
                curl
                ssl
                crypto
                z
        )
endif()

