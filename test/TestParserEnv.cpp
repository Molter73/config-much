#include "internal/parser-env.h"

#include <gtest/gtest.h>
#include <vector>

namespace ConfigMuch::internal {
TEST(ParserEnvTests, CamelToSnakeCase) {
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
        std::string processed = ParserEnv::camel_to_snake_case(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}

TEST(ParserEnvTests, ToUpper) {
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
        std::string processed = ParserEnv::to_upper(test.input);
        ASSERT_EQ(processed, test.expected);
    }
}

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

    for (const auto& test: tests) {
        std::string processed = ParserEnv::cook_env_var(test.prefix, test.suffix);
        ASSERT_EQ(processed, test.expected);
    }
}
} // namespace ConfigMuch
