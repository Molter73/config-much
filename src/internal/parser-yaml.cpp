#include "internal/parser-yaml.h"

namespace config_much::internal {
void ParserYaml::parse(google::protobuf::Message* msg) {
    using namespace google::protobuf;

    YAML::Node node = YAML::LoadFile(file_);
    parse(msg, node);
}

void ParserYaml::parse(google::protobuf::Message* msg, const YAML::Node& node) {
    using namespace google::protobuf;

    const Descriptor* descriptor = msg->GetDescriptor();
    for (int i = 0; i < descriptor->field_count(); i++) {
        const FieldDescriptor* field = descriptor->field(i);
        parse(msg, node, field);
    }
}

// Static helpers
namespace {
template <typename T>
void parse_array_inner(google::protobuf::Message* msg, const YAML::Node& node,
                       const google::protobuf::FieldDescriptor* field) {
    auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<T>(msg, field);
    f.Clear();
    for (const auto& n : node) {
        f.Add(n.as<T>());
    }
}

void parse_array_enum(google::protobuf::Message* msg, const YAML::Node& node,
                      const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<int32>(msg, field);
    f.Clear();

    const EnumDescriptor* desc = field->enum_type();
    for (const auto& n : node) {
        const auto name = n.as<std::string_view>();
        f.Add(desc->FindValueByName(name)->number());
    }
}
}; // namespace

void ParserYaml::parse(google::protobuf::Message* msg, const YAML::Node& node,
                       const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    if (!node[field->name()]) {
        return;
    }

    if (field->label() == FieldDescriptor::LABEL_REPEATED) {
        if (!node[field->name()].IsSequence()) {
            std::cerr << "Type mismatch for '" << field->name() << "', expected sequence, got "
                      << node[field->name()].Type();
        }
        return parse_array(msg, node[field->name()], field);
    }

    if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
        const Reflection* reflection = msg->GetReflection();

        Message* m = reflection->MutableMessage(msg, field);

        const Descriptor* descriptor = m->GetDescriptor();
        for (int i = 0; i < descriptor->field_count(); i++) {
            const FieldDescriptor* f = descriptor->field(i);

            if (node[field->name()]) {
                parse(m, node[field->name()], f);
            }
        }
        return;
    }

    if (!node[field->name()].IsScalar()) {
        std::cerr << "Attempting to parse non-scalar field as scalar" << std::endl;
        return;
    }

    switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE:
        msg->GetReflection()->SetDouble(msg, field, node[field->name()].as<double>());
        break;
    case FieldDescriptor::TYPE_FLOAT:
        msg->GetReflection()->SetFloat(msg, field, node[field->name()].as<float>());
        break;
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
        msg->GetReflection()->SetInt64(msg, field, node[field->name()].as<int64_t>());
        break;
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64:
        msg->GetReflection()->SetUInt64(msg, field, node[field->name()].as<uint64_t>());
        break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32:
        msg->GetReflection()->SetUInt32(msg, field, node[field->name()].as<uint32_t>());
        break;
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
        msg->GetReflection()->SetInt32(msg, field, node[field->name()].as<int32_t>());
        break;
    case FieldDescriptor::TYPE_BOOL:
        msg->GetReflection()->SetBool(msg, field, node[field->name()].as<bool>());
        break;
    case FieldDescriptor::TYPE_STRING:
        msg->GetReflection()->SetString(msg, field, node[field->name()].as<std::string>());
        break;
    case FieldDescriptor::TYPE_BYTES:
        std::cerr << "Unsupported type BYTES" << std::endl;
        break;
    case FieldDescriptor::TYPE_ENUM: {
        const auto name = node[field->name()].as<std::string_view>();

        const EnumDescriptor* descriptor = field->enum_type();
        const EnumValueDescriptor* value = descriptor->FindValueByName(name);
        msg->GetReflection()->SetEnumValue(msg, field, value->number());
    } break;

    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP:
        std::cerr << "Unexpected type!" << std::endl;
    }
}

void ParserYaml::parse_array(google::protobuf::Message* msg, const YAML::Node& node,
                             const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    // mapping for repeated fields:
    // https://protobuf.dev/reference/cpp/api-docs/google.protobuf.message/#Reflection.GetRepeatedFieldRef.details
    switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
        parse_array_inner<int32>(msg, node, field);
        break;
    case FieldDescriptor::CPPTYPE_UINT32:
        parse_array_inner<uint32_t>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
        parse_array_inner<int64_t>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
        parse_array_inner<uint64_t>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
        parse_array_inner<double>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
        parse_array_inner<float>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
        parse_array_inner<bool>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
        parse_array_enum(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
        parse_array_inner<std::string>(msg, node, field);
        break;
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
        std::cerr << "Unsupport repeated type MESSAGE" << std::endl;
        break;
    }
}
} // namespace config_much::internal
