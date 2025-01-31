#include "internal/parser-yaml.h"

#include <yaml-cpp/exceptions.h>

#include <exception>

namespace config_much::internal {
ParserResult ParserYaml::parse(google::protobuf::Message* msg) {
    using namespace google::protobuf;

    YAML::Node node = YAML::LoadFile(file_);
    return parse(msg, node);
}

ParserResult ParserYaml::parse(google::protobuf::Message* msg, const YAML::Node& node) {
    using namespace google::protobuf;

    ParserErrors errors;

    const Descriptor* descriptor = msg->GetDescriptor();
    for (int i = 0; i < descriptor->field_count(); i++) {
        const FieldDescriptor* field = descriptor->field(i);

        auto err = parse(msg, node, field);
        if (err) {
            errors.insert(errors.end(), err->begin(), err->end());
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
    ParserErrors errors;
    auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<T>(msg, field);
    f.Clear();
    for (const auto& n : node) {
        auto value = try_convert<T>(n);
        if (!is_error(value)) {
            f.Add(std::get<T>(value));
        } else {
            errors.emplace_back(std::get<ParserError>(value));
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

    ParserErrors errors;
    auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<int32>(msg, field);
    f.Clear();

    const EnumDescriptor* desc = field->enum_type();
    for (const auto& n : node) {
        auto v = try_convert<std::string_view>(n);
        if (is_error(v)) {
            errors.emplace_back(std::get<ParserError>(v));
            continue;
        }
        const auto name = std::get<std::string_view>(v);

        const EnumValueDescriptor* value = desc->FindValueByName(name);
        if (value == nullptr) {
            ParserError err;
            err << file_ << ": Invalid enum value '" << name << "' for field " << field->name();
            errors.emplace_back(err);
            continue;
        }

        f.Add(value->number());
    }

    if (!errors.empty()) {
        return errors;
    }
    return {};
}

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
}; // namespace

ParserResult ParserYaml::parse(google::protobuf::Message* msg, const YAML::Node& node,
                               const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    if (!node[field->name()]) {
        return {};
    }

    if (field->label() == FieldDescriptor::LABEL_REPEATED) {
        if (!node[field->name()].IsSequence()) {
            ParserError err;
            YAML::NodeType::value type = node[field->name()].Type();
            err << file_ << ": Type mismatch for '" << field->name() << "' - expected Sequence, got "
                << node_type_to_string(type);
            return {{err}};
        }
        return parse_array(msg, node[field->name()], field);
    }

    if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
        if (!node[field->name()].IsMap()) {
            ParserError err;
            YAML::NodeType::value type = node[field->name()].Type();
            err << file_ << ": Type mismatch for '" << field->name() << "' - expected Map, got "
                << node_type_to_string(type);
            return {{err}};
        }
        ParserErrors errors;
        const Reflection* reflection = msg->GetReflection();

        Message* m = reflection->MutableMessage(msg, field);

        const Descriptor* descriptor = m->GetDescriptor();
        for (int i = 0; i < descriptor->field_count(); i++) {
            const FieldDescriptor* f = descriptor->field(i);

            auto err = parse(m, node[field->name()], f);
            if (err) {
                errors.insert(errors.end(), err->begin(), err->end());
            }
        }

        if (!errors.empty()) {
            return errors;
        }
        return {};
    }

    if (!node[field->name()].IsScalar()) {
        ParserError err;
        err << file_ << ": Attempting to parse non-scalar field as scalar";
        return {{err}};
    }

    switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE: {
        auto value = try_convert<double>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetDouble(msg, field, std::get<double>(value));
    } break;
    case FieldDescriptor::TYPE_FLOAT: {
        auto value = try_convert<float>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetFloat(msg, field, std::get<float>(value));
    } break;
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64: {
        auto value = try_convert<int64_t>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetInt64(msg, field, std::get<int64_t>(value));
    } break;
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64: {
        auto value = try_convert<uint64_t>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetUInt64(msg, field, std::get<uint64_t>(value));
    } break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32: {
        auto value = try_convert<uint32_t>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetUInt32(msg, field, std::get<uint32_t>(value));
    } break;
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32: {
        auto value = try_convert<int32_t>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetInt32(msg, field, std::get<int32_t>(value));
    } break;
    case FieldDescriptor::TYPE_BOOL: {
        auto value = try_convert<bool>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetBool(msg, field, std::get<bool>(value));

    } break;
    case FieldDescriptor::TYPE_STRING: {
        auto value = try_convert<std::string>(node[field->name()]);
        if (is_error(value)) {
            return {{std::get<ParserError>(value)}};
        }

        msg->GetReflection()->SetString(msg, field, std::get<std::string>(value));

    } break;
    case FieldDescriptor::TYPE_BYTES:
        std::cerr << "Unsupported type BYTES" << std::endl;
        break;
    case FieldDescriptor::TYPE_ENUM: {
        const auto name = node[field->name()].as<std::string_view>();

        const EnumDescriptor* descriptor = field->enum_type();
        const EnumValueDescriptor* value = descriptor->FindValueByName(name);
        if (value == nullptr) {
            ParserError err;
            err << file_ << ": Invalid enum value '" << name << "' for field " << field->name();
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
        std::stringstream ss;
        ss << "Unsupport repeated type MESSAGE";
        return {{ss.str()}};
    } break;
    default: {
        std::stringstream ss;
        ss << "Unknown type " << field->type_name();
        return {{ss.str()}};
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
