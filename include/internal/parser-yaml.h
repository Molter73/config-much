#pragma once

#include "internal/parser-interface.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>

namespace ConfigMuch::internal {
class ParserYaml : public ParserInterface {
public:
    ParserYaml(const std::filesystem::path& file) {
        node_ = YAML::LoadFile(file);
    }

    void parse(google::protobuf::Message* msg) override;

private:
    static void parse(google::protobuf::Message* msg, const YAML::Node& node,
                      const google::protobuf::FieldDescriptor* field);
    static void parse_array(google::protobuf::Message* msg, const YAML::Node& node,
                            const google::protobuf::FieldDescriptor* field);

    YAML::Node node_;
};
} // namespace ConfigMuch::internal
