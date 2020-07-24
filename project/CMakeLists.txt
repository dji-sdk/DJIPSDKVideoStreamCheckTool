cmake_minimum_required(VERSION 3.14)
project(stream_check_tool CXX)

### Set toolchain flags
set(CMAKE_C_FLAGS "-pthread -std=gnu99")
set(CMAKE_EXE_LINKER_FLAGS "-pthread")
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

add_definitions(-D_GNU_SOURCE)

# Build external projects
include(./external_x264.cmake)

## Build Libraries
execute_process(COMMAND chmod 777 build_ffmpeg.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../middlewares/FFmpeg)
execute_process(COMMAND ./build_ffmpeg.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../middlewares/FFmpeg)

include_directories(../out/ffmpeg/include)
include_directories(../out/x264/include)
include_directories(../application/inc)

link_directories(${CMAKE_CURRENT_LIST_DIR}/../out/ffmpeg/lib)
link_directories(${CMAKE_CURRENT_LIST_DIR}/../out/x264/lib)
link_libraries(avformat avdevice avcodec avutil x264)

## Srcs
file(GLOB_RECURSE APP_SRC ../application/src/*.cpp)

## Outputs
add_executable(${CMAKE_PROJECT_NAME}
        ${APP_SRC}
        )