cmake_minimum_required(VERSION 3.12)
if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
endif ()

set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE INTERNAL "" FORCE)

project(ffmpeg_tutorial)

function(TUTORIAL name)
    set(TUTORIAL_TARGET ${name})
    add_executable(${TUTORIAL_TARGET} ${CMAKE_HOME_DIRECTORY}/src/${TUTORIAL_TARGET}.cpp)
    target_link_libraries(${TUTORIAL_TARGET} PRIVATE FFmpeg)
    if (WIN32)
        install(TARGETS ${name}
                RUNTIME DESTINATION ${CMAKE_BINARY_DIR})
    endif ()

endfunction(TUTORIAL)

include_directories("include" )
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(docs)
add_subdirectory(tests)









