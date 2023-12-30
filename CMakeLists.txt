cmake_minimum_required(VERSION 3.27)
project(gh_wh_handler)

set(CMAKE_CXX_STANDARD 23)

add_executable(gh_wh_handler main.cpp)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(gh_wh_handler PRIVATE -Wall -Wextra -pedantic)
endif()
