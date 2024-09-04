#pragma once

#include "google/protobuf/message.h"
#include "yaml-cpp/yaml.h"

namespace ConfigMuch {
class Parser {
public:
    Parser(std::vector<std::filesystem::path>&& files, std::string&& env_var_prefix_)
        : files_(files), env_var_prefix_(env_var_prefix_) {}

    void parse(google::protobuf::Message* msg);

private:
    void parse(google::protobuf::Message* msg, const YAML::Node& node, const google::protobuf::FieldDescriptor* field);
    void parse(google::protobuf::Message* msg, const std::string& prefix,
               const google::protobuf::FieldDescriptor* field);

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
