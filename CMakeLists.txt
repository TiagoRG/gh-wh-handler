cmake_minimum_required(VERSION 3.20)

# Project name and version
project(gh-wh-handler VERSION 0.1.0)

# Detect architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  set(ARCH "aarch64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
  set(ARCH "armv7l")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
  set(ARCH "i686")
  # Add more architectures as needed
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

# Add include directories
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
include_directories(${INCLUDE_DIR})

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${INCLUDE_DIR})

# Add compilation flags
target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Werror)

# Set linker flags for static linking
set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS
    "-static -static-libgcc -static-libstdc++")

# Install the executable
set(SERVICE_EXECUTABLE "/services/gh-wh-handler/${EXECUTABLE_NAME}")
set(SERVICE_CONFIG "/services/gh-wh-handler/config.json")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/gh-wh-handler.service.in"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gh-wh-handler.service"
@ONLY)
install(CODE "file(MAKE_DIRECTORY /services/gh-wh-handler)")
install(TARGETS ${EXECUTABLE_NAME} DESTINATION /services/gh-wh-handler)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    /services/gh-wh-handler/${EXECUTABLE_NAME} /usr/bin/gh-wh-handler)")
install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gh-wh-handler.service"
    DESTINATION /etc/systemd/system)
install(CODE "execute_process(COMMAND systemctl daemon-reload)")
install(CODE "execute_process(COMMAND systemctl enable gh-wh-handler)")
install(CODE "execute_process(COMMAND systemctl start gh-wh-handler)")


if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake" IMMEDIATE @ONLY)
  add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake)
endif()
