cmake_minimum_required(VERSION 3.12)
if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
endif ()

set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE INTERNAL "" FORCE)

project(ffmpeg_tutorial)

add_subdirectory(libs/FFmpeg)


function(TUTORIAL name)
    set(TUTORIAL_TARGET ${name})
    add_executable(${TUTORIAL_TARGET} ${TUTORIAL_TARGET}.cpp)
    target_link_libraries(${TUTORIAL_TARGET} PRIVATE FFmpeg)
    if (WIN32)
        install(TARGETS ${name}
                RUNTIME DESTINATION  ${CMAKE_BINARY_DIR})
    endif ()

endfunction(TUTORIAL)

TUTORIAL(tutorial01)





