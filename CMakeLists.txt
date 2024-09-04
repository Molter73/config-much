# Minimum CMake required
cmake_minimum_required(VERSION 3.28.2)

# Project
project(config-much)

option(BUILD_EXAMPLE "Build the example binary." OFF)
option(USE_ASAN "Build with asan and ubsan." OFF)

# Always generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required protobuf package
find_package(protobuf CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_library(config-much STATIC ${PROJECT_SOURCE_DIR}/src/config-much.cpp)
target_include_directories(config-much PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(config-much PRIVATE -Wall -Wextra -Wpedantic -Werror)
if (USE_ASAN)
    target_compile_options(config-much PRIVATE -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
    target_link_options(config-much PRIVATE -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
endif()
target_link_libraries(config-much PRIVATE protobuf::libprotobuf yaml-cpp::yaml-cpp)

if(BUILD_EXAMPLE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example)
endif()
