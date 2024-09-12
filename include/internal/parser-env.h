#pragma once

#include "internal/parser-interface.h"

#include <gtest/gtest_prod.h>

namespace ConfigMuch::internal {
class ParserEnv : public ParserInterface {
public:
    ParserEnv(const std::string& prefix) : prefix_(to_upper(prefix)) {}

    void parse(google::protobuf::Message* msg) override;

private:
    FRIEND_TEST(ParserEnvTests, CamelToSnakeCase);
    FRIEND_TEST(ParserEnvTests, ToUpper);
    FRIEND_TEST(ParserEnvTests, CookEnvVar);

    void parse(google::protobuf::Message* msg, const std::string& prefix,
               const google::protobuf::FieldDescriptor* field);
    //
    // Transformation methods for Environment Variables
    static std::string cook_env_var(const std::string& prefix, const std::string& suffix);
    static std::string camel_to_snake_case(const std::string& s);
    static std::string to_upper(const std::string& s);

    std::string prefix_;
};
} // namespace ConfigMuch::internal
