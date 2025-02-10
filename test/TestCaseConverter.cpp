#include "internal/case-convert.h"

#include <gtest/gtest.h>

namespace config_much::case_convert {
TEST(CaseConvertTests, CamelToSnake) {
    struct test_case {
        std::string input;
        std::string expected;
    };

    std::vector<test_case> tests = {
        {"no_change", "no_change"},      {"camelCase", "camel_case"},
        {"PascalCase", "pascal_case"},   {"SomeWeIrDcAsE", "some_we_ir_dc_as_e"},
        {"ALLUPPER", "a_l_l_u_p_p_e_r"}, {"numbers01234", "numbers01234"},
    };

    for (const auto& test : tests) {
        std::string processed = camel_to_snake(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(CaseConvertTests, CamelToSnakeUpper) {
    struct test_case {
        std::string input;
        std::string expected;
    };

    std::vector<test_case> tests = {
        {"all_caps", "ALL_CAPS"},        {"camelCase", "CAMEL_CASE"},
        {"PascalCase", "PASCAL_CASE"},   {"SomeWeIrDcAsE", "SOME_WE_IR_DC_AS_E"},
        {"ALLUPPER", "A_L_L_U_P_P_E_R"}, {"numbers01234", "NUMBERS01234"},
    };

    for (const auto& test : tests) {
        std::string processed = camel_to_snake(test.input, true);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(CaseConvertTests, SnakeToCamel) {
    struct test_case {
        std::string input;
        std::string expected;
    };

    std::vector<test_case> tests = {
        {"noChange", "noChange"},         {"camel_case", "camelCase"},
        {"PascalCase", "PascalCase"},     {"some_we_ir_dc_as_e", "someWeIrDcAsE"},
        {"numbers01234", "numbers01234"},
    };

    for (const auto& test : tests) {
        std::string processed = snake_to_camel(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(CaseConvertTests, AllCaps) {
    struct test_case {
        std::string input;
        std::string expected;
    };

    std::vector<test_case> tests = {
        {"NO_CHANGE", "NO_CHANGE"},
        {"lowercase", "LOWERCASE"},
        {"MiXed", "MIXED"},
    };

    for (const auto& test : tests) {
        std::string processed = all_caps(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}
} // namespace config_much::case_convert
