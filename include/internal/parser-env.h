#pragma once

#include "internal/case-convert.h"
#include "internal/parser-interface.h"

#include <gtest/gtest_prod.h>

namespace config_much::internal {
class ParserEnv : public ParserInterface {
public:
    ParserEnv(const std::string& prefix) : prefix_(case_convert::all_caps(prefix)) {}

    ParserResult parse(google::protobuf::Message* msg) override;

private:
    FRIEND_TEST(ParserEnvTests, CamelToSnakeCase);
    FRIEND_TEST(ParserEnvTests, ToUpper);
    FRIEND_TEST(ParserEnvTests, CookEnvVar);

    static void parse(google::protobuf::Message* msg, const std::string& prefix,
                      const google::protobuf::FieldDescriptor* field);
    static void parse_array(google::protobuf::Message* msg, const std::string& prefix,
                            const google::protobuf::FieldDescriptor* field);

    // Transformation methods for Environment Variables
    static std::string cook_env_var(const std::string& prefix, const std::string& suffix);

    std::string prefix_;
};
} // namespace config_much::internal
