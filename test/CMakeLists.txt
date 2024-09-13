find_package(GTest CONFIG REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB TEST_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(testSrc ${TEST_SRCS})
    get_filename_component(testExe ${testSrc} NAME_WE)

    message(INFO "Adding ${testExe}")

    add_executable(${testExe} ${testSrc} proto/test-config.proto)
    target_link_libraries(${testExe} PRIVATE config-much GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    protobuf_generate(TARGET ${testExe})

    add_test(${testExe} ${testExe})
endforeach()

