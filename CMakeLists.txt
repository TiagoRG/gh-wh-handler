cmake_minimum_required(VERSION 3.20)

# Project name and version
project(gh-wh-handler VERSION 0.1.0)

# Determine the architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ARCH "aarch64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
    set(ARCH "armv7l")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
    set(ARCH "i686")
else()
    set(ARCH "unknown")
endif()

# Set the executable name
set(EXECUTABLE_NAME "gh-wh-handler.${ARCH}")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Add compilation flags
target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Werror)

# Set linker flags for static linking
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    LINK_FLAGS "-static -static-libgcc -static-libstdc++"
)

