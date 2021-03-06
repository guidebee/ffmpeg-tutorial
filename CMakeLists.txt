cmake_minimum_required(VERSION 3.12)

if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
endif ()
set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")


project(ffmpeg_tutorial)

# -------------------------------------------------------------
# library dependencies
find_package(SDL2 CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
add_subdirectory(libs)

# -------------------------------------------------------------
# header references
include_directories("include")

# -------------------------------------------------------------
# utility functions
function(TUTORIAL name)
    set(TUTORIAL_TARGET ${name})
    add_executable(${TUTORIAL_TARGET} ${CMAKE_HOME_DIRECTORY}/src/${TUTORIAL_TARGET}.cpp
            ${CMAKE_HOME_DIRECTORY}/src/common.cpp)
    target_link_libraries(${TUTORIAL_TARGET} PRIVATE FFmpeg)
    target_link_libraries(${TUTORIAL_TARGET} PRIVATE SDL2::SDL2 SDL2::SDL2main)
    target_link_libraries(${TUTORIAL_TARGET} PRIVATE ${OpenCV_LIBS})

    if (WIN32)
        install(TARGETS ${name}
                RUNTIME DESTINATION ${CMAKE_BINARY_DIR})
    endif ()

endfunction(TUTORIAL)

# -------------------------------------------------------------
# project sub directories
add_subdirectory(apps)
add_subdirectory(docs)
add_subdirectory(tests)











