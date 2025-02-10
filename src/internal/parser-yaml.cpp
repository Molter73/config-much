#include "internal/parser-yaml.h"
#include "internal/case-convert.h"

#include <yaml-cpp/exceptions.h>

#include <exception>

namespace config_much::internal {

// Static helpers
namespace {
std::string node_type_to_string(YAML::NodeType::value type) {
    // Don't add the default case so linters can warn about a missing type
    switch (type) {
    case YAML::NodeType::Null:
        return "Null";
    case YAML::NodeType::Undefined:
        return "Undefined";
    case YAML::NodeType::Scalar:
        return "Scalar";
    case YAML::NodeType::Sequence:
        return "Sequence";
    case YAML::NodeType::Map:
        return "Map";
    }
    return ""; // Unreachable
}

std::string concat_path(const std::string& path, const std::string& name) {
    auto out = path;
    if (!path.empty()) {
        out += ".";
    }
    out += name;
    return out;
}
}; // namespace

ParserResult ParserYaml::parse(google::protobuf::Message* msg) {
    using namespace google::protobuf;
    YAML::Node node;

    try {
        node = YAML::LoadFile(file_);
    } catch (const YAML::BadFile& e) {
        return {{wrap_error(e)}};
    } catch (const YAML::ParserException& e) {
        return {{wrap_error(e)}};
    }

    return parse(msg, node);
}

ParserResult ParserYaml::parse(google::protobuf::Message* msg, const YAML::Node& node) {
    using namespace google::protobuf;

    if (node.IsScalar() || node.IsNull()) {
        return {{"Invalid configuration: root node should be a map."}};
    }

    std::vector<ParserError> errors;

    const Descriptor* descriptor = msg->GetDescriptor();
    for (int i = 0; i < descriptor->field_count(); i++) {
        const FieldDescriptor* field = descriptor->field(i);
        std::string path;

        auto err = parse(msg, node, field, path);
        if (err) {
            errors.insert(errors.end(), err->begin(), err->end());
        }
    }

    if (validation_mode_ != PERMISSIVE) {
        auto res = find_unknown_fields(*msg, node);
        if (res) {
            errors.insert(errors.end(), res->begin(), res->end());
        }
    }

    if (!errors.empty()) {
        return errors;
    }
    return {};
}

template <typename T>
ParserResult ParserYaml::parse_array_inner(google::protobuf::Message* msg, const YAML::Node& node,
                                           const google::protobuf::FieldDescriptor* field) {
    std::vector<ParserError> errors;
    auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<T>(msg, field);
    f.Clear();
    for (const auto& n : node) {
        auto value = try_convert<T>(n);
        if (!is_error(value)) {
            f.Add(std::get<T>(value));
        } else {
            errors.emplace_back(std::move(std::get<ParserError>(value)));
        }
    }

    if (!errors.empty()) {
        return errors;
    }
    return {};
}

ParserResult ParserYaml::parse_array_enum(google::protobuf::Message* msg, const YAML::Node& node,
                                          const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    std::vector<ParserError> errors;
    auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<int32>(msg, field);
    f.Clear();

    const EnumDescriptor* desc = field->enum_type();
    for (const auto& n : node) {
        auto v = try_convert<std::string_view>(n);
        if (is_error(v)) {
            errors.emplace_back(std::move(std::get<ParserError>(v)));
            continue;
        }
        const auto name = std::get<std::string_view>(v);

        const EnumValueDescriptor* value = desc->FindValueByName(name);
        if (value == nullptr) {
            ParserError err;
            err << file_ << ": Invalid enum value '" << name << "' for field "
                << (camelcase_ ? case_convert::snake_to_camel(field->name()) : field->name());
            errors.emplace_back(std::move(err));
            continue;
        }

        f.Add(value->number());
    }

    if (!errors.empty()) {
        return errors;
    }
    return {};
}

ParserResult ParserYaml::find_unknown_fields(const google::protobuf::Message& msg, const YAML::Node& node) {
    using namespace google::protobuf;

    const auto* descriptor = msg.GetDescriptor();
    std::vector<ParserError> errors;

    for (YAML::const_iterator it = node.begin(); it != node.end(); it++) {
        auto name = it->first.as<std::string>();
        if (camelcase_) {
            name = case_convert::camel_to_snake(name);
        }

        const FieldDescriptor* field = descriptor->FindFieldByName(name);
        if (field == nullptr) {
            ParserError err;
            err << "Unknown field '" << name << "'";
            errors.emplace_back(std::move(err));
            continue;
        }

        if (it->second.IsMap()) {
            if (field->type() != FieldDescriptor::TYPE_MESSAGE) {
                ParserError err;
                err << file_ << ": Invalid type '" << node_type_to_string(it->second.Type()) << "' for field "
                    << it->first.as<std::string_view>() << ", expected '" << field->type_name() << "'";
                errors.emplace_back(std::move(err));
                continue;
            }

            const auto* reflection = msg.GetReflection();
            auto res               = find_unknown_fields(reflection->GetMessage(msg, field), it->second);

            if (res) {
                errors.insert(errors.end(), res->begin(), res->end());
            }
        }
    }

    if (!errors.empty()) {
        return errors;
    }
    return {};
}

ParserResult ParserYaml::parse(google::protobuf::Message* msg, const YAML::Node& node,
                               const google::protobuf::FieldDescriptor* field, const std::string& path) {
    using namespace google::protobuf;

    std::unique_ptr<std::string> name_ptr = nullptr;
    const std::string* name               = &field->name();
    if (camelcase_) {
        name_ptr = std::make_unique<std::string>(case_convert::snake_to_camel(*name));
        name     = name_ptr.get();
    }

    if (!node[*name]) {
        if (validation_mode_ == STRICT) {
            ParserError err;
            err << "Missing field '" << concat_path(path, *name) << "'";
            return {{err}};
        }
        return {};
    }

    if (field->label() == FieldDescriptor::LABEL_REPEATED) {
        if (!node[*name].IsSequence()) {
            ParserError err;
            YAML::NodeType::value type = node[*name].Type();
            err << file_ << ": Type mismatch for '" << *name << "' - expected Sequence, got "
                << node_type_to_string(type);
            return {{err}};
        }
        return parse_array(msg, node[*name], field);
    }

    if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
        if (!node[*name].IsMap()) {
            ParserError err;
            YAML::NodeType::value type = node[*name].Type();
            err << file_ << ": Type mismatch for '" << *name << "' - expected Map, got " << node_type_to_string(type);
            return {{err}};
        }
        std::vector<ParserError> errors;
        const Reflection* reflection = msg->GetReflection();

        Message* m = reflection->MutableMessage(msg, field);

        const Descriptor* descriptor = m->GetDescriptor();
        for (int i = 0; i < descriptor->field_count(); i++) {
            const FieldDescriptor* f = descriptor->field(i);

            auto err = parse(m, node[*name], f, concat_path(path, *name));
            if (err) {
                errors.insert(errors.end(), err->begin(), err->end());
            }
        }

        if (!errors.empty()) {
            return errors;
        }
        return {};
    }

    if (!node[*name].IsScalar()) {
        ParserError err;
        err << file_ << ": Attempting to parse non-scalar field as scalar";
        return {{err}};
    }

    return parse_scalar(msg, node[*name], field, *name);
}

ParserResult ParserYaml::parse_scalar(google::protobuf::Message* msg, const YAML::Node& node,
                                      const google::protobuf::FieldDescriptor* field, const std::string& name) {
    using namespace google::protobuf;

    switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE: {
        auto value = try_convert<double>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetDouble(msg, field, std::get<double>(value));
    } break;
    case FieldDescriptor::TYPE_FLOAT: {
        auto value = try_convert<float>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetFloat(msg, field, std::get<float>(value));
    } break;
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64: {
        auto value = try_convert<int64_t>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetInt64(msg, field, std::get<int64_t>(value));
    } break;
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64: {
        auto value = try_convert<uint64_t>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetUInt64(msg, field, std::get<uint64_t>(value));
    } break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32: {
        auto value = try_convert<uint32_t>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetUInt32(msg, field, std::get<uint32_t>(value));
    } break;
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32: {
        auto value = try_convert<int32_t>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetInt32(msg, field, std::get<int32_t>(value));
    } break;
    case FieldDescriptor::TYPE_BOOL: {
        auto value = try_convert<bool>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetBool(msg, field, std::get<bool>(value));

    } break;
    case FieldDescriptor::TYPE_STRING: {
        auto value = try_convert<std::string>(node);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetString(msg, field, std::get<std::string>(value));

    } break;
    case FieldDescriptor::TYPE_BYTES:
        return {{"Unsupported type BYTES"}};
    case FieldDescriptor::TYPE_ENUM: {
        // We assume enum definitions use UPPER_CASE
        const auto enum_name = case_convert::all_caps(node.as<std::string>());

        const EnumDescriptor* descriptor = field->enum_type();
        const EnumValueDescriptor* value = descriptor->FindValueByName(enum_name);
        if (value == nullptr) {
            ParserError err;
            err << file_ << ": Invalid enum value '" << enum_name << "' for field " << name;
            return {{err}};
        }
        msg->GetReflection()->SetEnumValue(msg, field, value->number());
    } break;

    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP: {
        ParserError err;
        err << "Unexpected type: " << field->type_name();
        return {{err}};
    }
    }

    return {};
}

ParserResult ParserYaml::parse_array(google::protobuf::Message* msg, const YAML::Node& node,
                                     const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    // mapping for repeated fields:
    // https://protobuf.dev/reference/cpp/api-docs/google.protobuf.message/#Reflection.GetRepeatedFieldRef.details
    switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
        return parse_array_inner<int32>(msg, node, field);
    case FieldDescriptor::CPPTYPE_UINT32:
        return parse_array_inner<uint32_t>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
        return parse_array_inner<int64_t>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
        return parse_array_inner<uint64_t>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
        return parse_array_inner<double>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
        return parse_array_inner<float>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
        return parse_array_inner<bool>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
        return parse_array_enum(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
        return parse_array_inner<std::string>(msg, node, field);
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        return {{"Unsupport repeated type MESSAGE"}};
    } break;
    default: {
        ParserError err;
        err << "Unknown type " << field->type_name();
        return {{err}};
    }
    }
}

ParserError ParserYaml::wrap_error(const std::exception& e) {
    std::stringstream ss;
    ss << file_ << ": " << e.what();
    return ss.str();
}

template <typename T> std::variant<T, ParserError> ParserYaml::try_convert(const YAML::Node& node) {
    try {
        return node.as<T>();
    } catch (YAML::InvalidNode& e) {
        return wrap_error(e);
    } catch (YAML::BadConversion& e) {
        return wrap_error(e);
    }
}
} // namespace config_much::internal
