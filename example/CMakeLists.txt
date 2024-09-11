set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(executable_name test-config)
add_executable(${executable_name} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/config.proto)
target_link_libraries(${executable_name} protobuf::libprotobuf config-much)
protobuf_generate(TARGET ${executable_name})
