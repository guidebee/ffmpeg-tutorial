cmake_minimum_required(VERSION 3.12)
project(SDL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

find_path(SDL_INCLUDE_DIR SDL2/SDL.h)
find_library(SDL_LIBRARY SDL2)


add_library(SDL INTERFACE)

target_include_directories(SDL INTERFACE
        ${SDL_INCLUDE_DIR})

target_link_libraries(SDL INTERFACE
        ${SDL_LIBRARY}
        )
