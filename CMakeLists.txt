# Minimum CMake required
cmake_minimum_required(VERSION 3.28.2)

# Project
project(config-much)

option(BUILD_TESTS "Build unit tests." OFF)
option(BUILD_EXAMPLE "Build the example binary." OFF)
option(USE_ASAN "Build with asan and ubsan." OFF)

# Always generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required protobuf package
find_package(protobuf CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

if (USE_ASAN)
    add_compile_options(-fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
    add_link_options(-fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined)
endif()
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_library(config-much STATIC ${PROJECT_SOURCE_DIR}/src/config-much.cpp)
target_include_directories(config-much PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(config-much PRIVATE protobuf::libprotobuf yaml-cpp::yaml-cpp)

if(BUILD_TESTS)
    enable_testing()

    add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()

if(BUILD_EXAMPLE)
    add_subdirectory(${PROJECT_SOURCE_DIR}/example)
endif()
