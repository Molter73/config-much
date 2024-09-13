#include "internal/parser-yaml.h"

#include "proto/test-config.pb.h"

#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <vector>

namespace config_much::internal {
using namespace google::protobuf::util;

TEST(TestParserYaml, Parsing) {
    struct test_case {
        std::string input;
        test_config::Config expected;
    };

    test_config::Config all_fields;
    all_fields.set_enabled(true);
    all_fields.set_field_i32(-32);
    all_fields.set_field_u32(32);
    all_fields.set_field_i64(-64);
    all_fields.set_field_u64(64);
    all_fields.set_field_double(3.14);
    all_fields.set_field_float(0.12345);
    all_fields.set_field_string("Yes, this is some random string for testing");
    all_fields.mutable_field_message()->set_enabled(true);
    all_fields.mutable_field_repeated()->Add(1);
    all_fields.mutable_field_repeated()->Add(2);
    all_fields.mutable_field_repeated()->Add(3);

    std::vector<test_case> tests = {
        {R"()", {}},
        {R"(
            enabled: true
            field_i32: -32
            field_u32: 32
            field_i64: -64
            field_u64: 64
            field_double: 3.14
            field_float: 0.12345
            field_string: Yes, this is some random string for testing
            field_message:
                enabled: true
            field_repeated:
                - 1
                - 2
                - 3
        )",
         all_fields},
    };

    for (const auto& [input, expected] : tests) {
        test_config::Config parsed;
        ParserYaml::parse(&parsed, YAML::Load(input));

        bool equals = MessageDifferencer::Equals(parsed, expected);

        ASSERT_TRUE(equals) << "### parsed: " << std::endl
                            << parsed.DebugString() << std::endl
                            << "### expected: " << std::endl
                            << expected.DebugString();
    }
}

TEST(TestParserYaml, OverwrittingFields) {
    test_config::Config cfg;
    std::string input = R"(
        enabled: false
        field_i32: -1234
        field_u32: 4321
        field_repeated:
            - 15
    )";

    test_config::Config expected;
    expected.set_enabled(false);
    expected.set_field_i32(-1234);
    expected.set_field_u32(4321);
    expected.mutable_field_repeated()->Add(15);
    ParserYaml::parse(&cfg, YAML::Load(input));

    bool equals = MessageDifferencer::Equals(cfg, expected);
    ASSERT_TRUE(equals) << "### parsed: " << std::endl
                        << cfg.DebugString() << std::endl
                        << "### expected: " << std::endl
                        << expected.DebugString();

    input = R"(
        enabled: true
        field_u32: 1234
        field_repeated:
            - 1
            - 2
            - 3
    )";

    expected.set_enabled(true);
    expected.set_field_u32(1234);
    expected.mutable_field_repeated()->Clear();
    expected.mutable_field_repeated()->Add(1);
    expected.mutable_field_repeated()->Add(2);
    expected.mutable_field_repeated()->Add(3);

    ParserYaml::parse(&cfg, YAML::Load(input));

    equals = MessageDifferencer::Equals(cfg, expected);
    ASSERT_TRUE(equals) << "### parsed: " << std::endl
                        << cfg.DebugString() << std::endl
                        << "### expected: " << std::endl
                        << expected.DebugString();
}
} // namespace config_much::internal
