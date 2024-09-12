find_package(GTest CONFIG REQUIRED)

add_executable(TestParserEnv TestParserEnv.cpp)
target_link_libraries(TestParserEnv PRIVATE config-much GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(TestParserEnv TestParserEnv)
