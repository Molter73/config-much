#pragma once

#include <google/protobuf/message.h>
#include <gtest/gtest_prod.h>
#include <yaml-cpp/yaml.h>

namespace ConfigMuch {
class Parser {
public:
    Parser(std::vector<std::filesystem::path>&& files, std::string&& env_var_prefix_)
        : files_(files), env_var_prefix_(env_var_prefix_) {}

    void parse(google::protobuf::Message* msg);

private:
    FRIEND_TEST(ParserTests, CamelToSnakeCase);
    FRIEND_TEST(ParserTests, ToUpper);
    FRIEND_TEST(ParserTests, CookEnvVar);

    void parse(google::protobuf::Message* msg, const YAML::Node& node, const google::protobuf::FieldDescriptor* field);
    void parse(google::protobuf::Message* msg, const std::string& prefix,
               const google::protobuf::FieldDescriptor* field);

    template <typename T>
    void parse_array_field(google::protobuf::Message* msg, const YAML::Node& node,
                           const google::protobuf::FieldDescriptor* field);
    void parse_array(google::protobuf::Message* msg, const YAML::Node& node,
                     const google::protobuf::FieldDescriptor* field);

    // Transformation methods for Environment Variables
    static std::string cook_env_var(const std::string& prefix, const std::string& suffix);
    static std::string camel_to_snake_case(const std::string& s);
    static std::string to_upper(const std::string& s);

    std::vector<std::filesystem::path> files_;
    std::string env_var_prefix_;
};

class Builder {
public:
    Builder& add_file(const std::filesystem::path& p);
    Builder& set_env_var_prefix(const std::string& prefix) {
        env_var_prefix_ = prefix;
        return *this;
    }

    Parser build() { return {std::move(files_), std::move(env_var_prefix_)}; }

private:
    std::string env_var_prefix_;
    std::vector<std::filesystem::path> files_;
};
} // namespace ConfigMuch
