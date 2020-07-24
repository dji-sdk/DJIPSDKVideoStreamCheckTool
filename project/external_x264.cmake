include(FetchContent)

set(X264_GIT_TAG  master)
if (NOT X264_GIT_URL)
    set(X264_GIT_URL https://code.videolan.org/videolan/x264.git)
endif (NOT X264_GIT_URL)

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/../middlewares/x264)

FetchContent_Declare(
        x264
        GIT_REPOSITORY    ${X264_GIT_URL}
        GIT_TAG           ${X264_GIT_TAG}
)

FetchContent_MakeAvailable(x264)

execute_process(COMMAND cp build_x264.sh ${CMAKE_SOURCE_DIR}/../middlewares/x264/x264-src WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
execute_process(COMMAND chmod 777 build_x264.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../middlewares/x264/x264-src)
execute_process(COMMAND ./build_x264.sh WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../middlewares/x264/x264-src)
