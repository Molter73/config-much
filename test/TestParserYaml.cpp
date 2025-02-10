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
        bool camelcase;
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
    all_fields.set_field_enum(test_config::EnumField::TYPE2);
    all_fields.mutable_field_repeated_enum()->Add(0);
    all_fields.mutable_field_repeated_enum()->Add(1);

    std::vector<test_case> tests = {
        {R"()", {}, false},
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
            field_enum: TYPE2
            field_repeated_enum:
                - TYPE1
                - TYPE2
        )",
         all_fields, false},
        {R"(
            enabled: true
            fieldI32: -32
            fieldU32: 32
            fieldI64: -64
            fieldU64: 64
            fieldDouble: 3.14
            fieldFloat: 0.12345
            fieldString: Yes, this is some random string for testing
            fieldMessage:
                enabled: true
            fieldRepeated:
                - 1
                - 2
                - 3
            fieldEnum: TYPE2
            fieldRepeatedEnum:
                - TYPE1
                - TYPE2
        )",
         all_fields, true},
    };

    for (const auto& [input, expected, camelcase] : tests) {
        test_config::Config parsed;
        ParserYaml parser("/test.yml", camelcase);
        parser.parse(&parsed, YAML::Load(input));

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
        field_enum: TYPE1
        field_repeated_enum:
            - TYPE1
    )";

    test_config::Config expected;
    expected.set_enabled(false);
    expected.set_field_i32(-1234);
    expected.set_field_u32(4321);
    expected.mutable_field_repeated()->Add(15);
    expected.set_field_enum(test_config::EnumField::TYPE1);
    expected.mutable_field_repeated_enum()->Add(0);
    ParserYaml parser("/test.yml");
    parser.parse(&cfg, YAML::Load(input));

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
        field_enum: TYPE2
        field_repeated_enum:
            - TYPE2
            - TYPE2
    )";

    expected.set_enabled(true);
    expected.set_field_u32(1234);
    expected.mutable_field_repeated()->Clear();
    expected.mutable_field_repeated()->Add(1);
    expected.mutable_field_repeated()->Add(2);
    expected.mutable_field_repeated()->Add(3);
    expected.set_field_enum(test_config::EnumField::TYPE2);
    expected.mutable_field_repeated_enum()->Clear();
    expected.mutable_field_repeated_enum()->Add(1);
    expected.mutable_field_repeated_enum()->Add(1);

    parser.parse(&cfg, YAML::Load(input));

    equals = MessageDifferencer::Equals(cfg, expected);
    ASSERT_TRUE(equals) << "### parsed: " << std::endl
                        << cfg.DebugString() << std::endl
                        << "### expected: " << std::endl
                        << expected.DebugString();
}

TEST(TestParserYaml, ParserErrors) {
    test_config::Config cfg;
    ParserYaml parser("/test.yml");
    const std::string input = R"(
        enabled: 1
        field_i32: wrong
        field_u32: {}
        field_i64: also_wrong
        field_u64: -64
        field_double: {}
        field_float: {}
        field_string: 123
        field_message: 1.2
        field_repeated: 1
        field_enum: NOT_REAL
        field_repeated_enum:
            - NOT_REAL
            - ALSO_INVALID
            - TYPE2
    )";

    const std::vector<ParserError> expected = {
        "\"/test.yml\": yaml-cpp: error at line 2, column 18: bad conversion",
        "\"/test.yml\": yaml-cpp: error at line 3, column 20: bad conversion",
        "\"/test.yml\": Attempting to parse non-scalar field as scalar",
        "\"/test.yml\": yaml-cpp: error at line 5, column 20: bad conversion",
        "\"/test.yml\": yaml-cpp: error at line 6, column 20: bad conversion",
        "\"/test.yml\": Attempting to parse non-scalar field as scalar",
        "\"/test.yml\": Attempting to parse non-scalar field as scalar",
        "\"/test.yml\": Type mismatch for 'field_message' - expected Map, got Scalar",
        "\"/test.yml\": Type mismatch for 'field_repeated' - expected Sequence, got Scalar",
        "\"/test.yml\": Invalid enum value 'NOT_REAL' for field field_enum",
        "\"/test.yml\": Invalid enum value 'NOT_REAL' for field field_repeated_enum",
        "\"/test.yml\": Invalid enum value 'ALSO_INVALID' for field field_repeated_enum",
        "\"/test.yml\": Invalid type 'Map' for field field_u32, expected 'uint32'",
        "\"/test.yml\": Invalid type 'Map' for field field_double, expected 'double'",
        "\"/test.yml\": Invalid type 'Map' for field field_float, expected 'float'",
    };

    auto errors = parser.parse(&cfg, YAML::Load(input));
    ASSERT_TRUE(errors);
    ASSERT_EQ(errors->size(), expected.size());

    for (unsigned int i = 0; i < expected.size(); i++) {
        ASSERT_EQ(errors->at(i), expected.at(i));
    }
}
} // namespace config_much::internal
