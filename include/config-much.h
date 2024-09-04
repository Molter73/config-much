#pragma once

#include "google/protobuf/message.h"
#include "yaml-cpp/yaml.h"

namespace ConfigMuch {
class Parser {
public:
    void add_file(const std::filesystem::path& p);
    void parse(google::protobuf::Message* msg);
    void set_env_var_prefix(const std::string& prefix) { env_var_prefix_ = prefix; }

private:
    void parse(google::protobuf::Message* msg, const YAML::Node& node, const google::protobuf::FieldDescriptor* field);
    void parse(google::protobuf::Message* msg, const std::string& prefix,
               const google::protobuf::FieldDescriptor* field);

    std::vector<std::filesystem::path> files_;
    std::string env_var_prefix_;
};
} // namespace ConfigMuch
