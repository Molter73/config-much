#pragma once

#include "internal/parser-interface.h"

#include <yaml-cpp/yaml.h>

#include <exception>
#include <filesystem>

namespace config_much::internal {
class ParserYaml : public ParserInterface {
public:
    ParserYaml(std::filesystem::path file, bool camelcase = false) : file_(std::move(file)), camelcase_(camelcase) {}

    ParserResult parse(google::protobuf::Message* msg) override;
    ParserResult parse(google::protobuf::Message* msg, const YAML::Node& node);

    const std::filesystem::path& get_file() { return file_; }

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
    ParserResult parse_scalar(google::protobuf::Message* msg, const YAML::Node& node,
                              const google::protobuf::FieldDescriptor* field, const std::string& name);

    ParserResult find_unknown_fields(const google::protobuf::Message& msg, const YAML::Node& node);

    ParserError wrap_error(const std::exception& e);

    template <typename T> std::variant<T, ParserError> try_convert(const YAML::Node& node);
    template <typename T> bool is_error(const std::variant<T, ParserError>& res) {
        return std::holds_alternative<ParserError>(res);
    }

    std::filesystem::path file_;
    bool camelcase_;
};
} // namespace config_much::internal
