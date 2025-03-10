#include "internal/parser-env.h"
#include "proto/test-config.pb.h"

#include <cstdlib>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>
#include <vector>

namespace config_much::internal {
using namespace google::protobuf::util;

TEST(ParserEnvTests, CookEnvVar) {
    struct test_case {
        std::string prefix;
        std::string suffix;
        std::string expected;
    };

    std::vector<test_case> tests = {
        {"MY_APP", "FeatureOne", "MY_APP_FEATURE_ONE"},
        {"prefix", "some_feature", "prefix_SOME_FEATURE"},
        {"MY_APP", "mixed_cAse", "MY_APP_MIXED_C_ASE"},
    };

    for (const auto& test : tests) {
        std::string processed = ParserEnv::cook_env_var(test.prefix, test.suffix);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(ParserEnvTests, Parsing) {
    test_config::Config expected;
    expected.set_enabled(true);
    expected.set_field_i32(-32);
    expected.set_field_u32(32);
    expected.set_field_i64(-64);
    expected.set_field_u64(64);
    expected.set_field_double(3.14);
    expected.set_field_float(0.12345);
    expected.set_field_string("Yes, this is some random string for testing");
    expected.mutable_field_message()->set_enabled(true);
    expected.mutable_field_repeated()->Add(1);
    expected.mutable_field_repeated()->Add(2);
    expected.mutable_field_repeated()->Add(3);
    expected.set_field_enum(test_config::TYPE2);
    expected.mutable_field_repeated_enum()->Add(0);
    expected.mutable_field_repeated_enum()->Add(1);

    setenv("MY_APP_ENABLED", "true", 0);
    setenv("MY_APP_FIELD_I32", "-32", 0);
    setenv("MY_APP_FIELD_U32", "32", 0);
    setenv("MY_APP_FIELD_I64", "-64", 0);
    setenv("MY_APP_FIELD_U64", "64", 0);
    setenv("MY_APP_FIELD_DOUBLE", "3.14", 0);
    setenv("MY_APP_FIELD_FLOAT", "0.12345", 0);
    setenv("MY_APP_FIELD_STRING", "Yes, this is some random string for testing", 0);
    setenv("MY_APP_FIELD_MESSAGE_ENABLED", "true", 0);
    setenv("MY_APP_FIELD_REPEATED_0", "1", 0);
    setenv("MY_APP_FIELD_REPEATED_1", "2", 0);
    setenv("MY_APP_FIELD_REPEATED_2", "3", 0);
    setenv("MY_APP_FIELD_ENUM", "TYPE2", 0);
    setenv("MY_APP_FIELD_REPEATED_ENUM_0", "TYPE1", 0);
    setenv("MY_APP_FIELD_REPEATED_ENUM_1", "TYPE2", 0);

    test_config::Config parsed;
    ParserEnv{"MY_APP"}.parse(&parsed);

    bool equals = MessageDifferencer::Equals(parsed, expected);
    ASSERT_TRUE(equals) << "### parsed: " << std::endl
                        << parsed.DebugString() << std::endl
                        << "### expected: " << std::endl
                        << expected.DebugString();
}
} // namespace config_much::internal
