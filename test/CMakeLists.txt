enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(TestParser TestParser.cpp)
target_link_libraries(TestParser PRIVATE config-much GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(TestParser TestParser)
