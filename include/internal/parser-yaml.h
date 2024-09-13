#pragma once

#include "internal/parser-interface.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>

namespace config_much::internal {
class ParserYaml : public ParserInterface {
public:
    ParserYaml(std::filesystem::path file) : file_(std::move(file)) {}

    void parse(google::protobuf::Message* msg) override;

private:
    static void parse(google::protobuf::Message* msg, const YAML::Node& node,
                      const google::protobuf::FieldDescriptor* field);
    static void parse_array(google::protobuf::Message* msg, const YAML::Node& node,
                            const google::protobuf::FieldDescriptor* field);

    std::filesystem::path file_;
};
} // namespace config_much::internal
