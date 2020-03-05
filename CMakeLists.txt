cmake_minimum_required(VERSION 3.12)

project(ffmpeg_tutorial)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY avfilter)

find_path(AVRESAMPLE_INCLUDE_DIR libavresample/avresample.h)
find_library(AVRESAMPLE_LIBRARY avresample)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY swresample)


set(FFMPEG_INCLUDE_DIR ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${AVDEVICE_INCLUDE_DIR}
        ${SWSCALE_INCLUDE_DIR}
        ${AVFILTER_INCLUDE_DIR}
        ${AVRESAMPLE_INCLUDE_DIR}
        ${SWRESAMPLE_INCLUDE_DIR})

set(FFMPEG_LIBRARY ${AVCODEC_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${AVDEVICE_LIBRARY}
        ${SWSCALE_LIBRARY}
        ${AVFILTER_LIBRARY}
        ${AVRESAMPLE_LIBRARY}
        ${SWRESAMPLE_LIBRARY})


SET(VCPKG_ROOT_BIN_DIR ${_VCPKG_ROOT_DIR}/installed/x64-windows/bin)
add_custom_target(copy_dlls
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/avcodec-58.dll       ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/avdevice-58.dll      ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/avfilter-7.dll       ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/avformat-58.dll      ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/avresample-4.dll     ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/avutil-56.dll        ${CMAKE_BINARY_DIR}/

        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/swresample-3.dll     ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/swscale-5.dll        ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/SDL2.dll             ${CMAKE_BINARY_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${VCPKG_ROOT_BIN_DIR}/SDL2.dll             ${CMAKE_BINARY_DIR}/
)


function(TUTORIAL name)
    set(TUTORIAL_TARGET ${name})
    add_executable(${TUTORIAL_TARGET} ${TUTORIAL_TARGET}.cpp)
    target_include_directories(${TUTORIAL_TARGET} PRIVATE ${FFMPEG_INCLUDE_DIR})
    target_link_libraries(${TUTORIAL_TARGET} PRIVATE ${FFMPEG_LIBRARY})
    if(WIN32)
        add_dependencies(${name} copy_dlls)
    endif()
endfunction(TUTORIAL)


TUTORIAL(tutorial01)

