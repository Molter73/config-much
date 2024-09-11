#include "config-much.h"

#include <gtest/gtest.h>
#include <vector>

namespace ConfigMuch {
TEST(ParserTests, CamelToSnakeCase) {
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
        std::string processed = ConfigMuch::Parser::camel_to_snake_case(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(ParserTests, ToUpper) {
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
        std::string processed = ConfigMuch::Parser::to_upper(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(ParserTests, CookEnvVar) {
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

    for (const auto& test: tests) {
        std::string processed = ConfigMuch::Parser::cook_env_var(test.prefix, test.suffix);
        ASSERT_EQ(processed, test.expected);
    }
}
} // namespace ConfigMuch
