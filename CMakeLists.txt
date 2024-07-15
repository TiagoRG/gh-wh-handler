cmake_minimum_required(VERSION 3.20)

# Project name and version
project(gh-wh-handler VERSION 0.1.0)

# Set the executable name
set(EXECUTABLE_NAME "gh-wh-handler")

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
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    LINK_FLAGS "-static -static-libgcc -static-libstdc++"
)

# Install the service
set(CMAKE_INSTALL_PREFIX "/usr/")
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
set(SERVICE_EXECUTABLE "/services/gh-wh-handler/gh-wh-handler.aarch64")
set(SERVICE_CONFIG_FILE "/services/gh-wh-handler/config.json")
configure_file(${CMAKE_SOURCE_DIR}/gh-wh-handler.service.in ${CMAKE_BINARY_DIR}/gh-wh-handler.service @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/gh-wh-handler.service DESTINATION /etc/systemd/system)
install(CODE "
    execute_process(COMMAND systemctl daemon-reload)
    execute_process(COMMAND systemctl enable gh-wh-handler.service)
    execute_process(COMMAND systemctl start gh-wh-handler.service)
")

