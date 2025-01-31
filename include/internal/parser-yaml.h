#pragma once

#include "internal/parser-interface.h"

#include <yaml-cpp/yaml.h>

#include <exception>
#include <filesystem>

namespace config_much::internal {
class ParserYaml : public ParserInterface {
public:
    ParserYaml(std::filesystem::path file) : file_(std::move(file)) {}

    ParserResult parse(google::protobuf::Message* msg) override;
    ParserResult parse(google::protobuf::Message* msg, const YAML::Node& node);

private:
    ParserResult parse(google::protobuf::Message* msg, const YAML::Node& node,
                       const google::protobuf::FieldDescriptor* field);
    ParserResult parse_array(google::protobuf::Message* msg, const YAML::Node& node,
                             const google::protobuf::FieldDescriptor* field);
    template <typename T>
    ParserResult parse_array_inner(google::protobuf::Message* msg, const YAML::Node& node,
                                   const google::protobuf::FieldDescriptor* field);
    ParserResult parse_array_enum(google::protobuf::Message* msg, const YAML::Node& node,
                                  const google::protobuf::FieldDescriptor* field);

    ParserError wrap_error(const std::exception& e);

    template <typename T> std::variant<T, ParserError> try_convert(const YAML::Node& node);
    template <typename T> bool is_error(const std::variant<T, ParserError>& res) {
        return std::holds_alternative<ParserError>(res);
    }

    std::filesystem::path file_;
};
} // namespace config_much::internal
