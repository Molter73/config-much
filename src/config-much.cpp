#include "config-much.h"

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

namespace ConfigMuch {
Builder& Builder::add_file(const std::filesystem::path& p) {
    files_.emplace_back(p);
    return *this;
}

void Parser::parse(google::protobuf::Message* msg) {
    using namespace google::protobuf;

    const Descriptor* descriptor = msg->GetDescriptor();
    for (const auto& file : files_) {
        YAML::Node node = YAML::LoadFile(file);

        for (int i = 0; i < descriptor->field_count(); i++) {
            const FieldDescriptor* field = descriptor->field(i);
            parse(msg, node, field);
        }
    }

    // Now we load Environment Variables
    for (int i = 0; i < descriptor->field_count(); i++) {
        const FieldDescriptor* field = descriptor->field(i);
        parse(msg, env_var_prefix_, field);
    }
}

void Parser::parse(google::protobuf::Message* msg, const YAML::Node& node,
                   const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
        std::cout << "Parsing field: " << field->name() << std::endl;
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

    if (!node[field->name()]) {
        std::cout << "Skip missing field: " << field->name() << std::endl;
        return;
    }

    if (!node[field->name()].IsScalar()) {
        std::cerr << "Attempting to parse non-scalar field as scalar" << std::endl;
        return;
    }

    switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE: {
        auto value = node[field->name()].as<double>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetDouble(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_FLOAT: {
        auto value = node[field->name()].as<float>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetFloat(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64: {
        auto value = node[field->name()].as<int64_t>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetInt64(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64: {
        auto value = node[field->name()].as<uint64_t>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetUInt64(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32: {
        auto value = node[field->name()].as<uint32_t>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetUInt32(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32: {
        auto value = node[field->name()].as<int32_t>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetInt32(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_BOOL: {
        auto value = node[field->name()].as<bool>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetBool(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_STRING: {
        auto value = node[field->name()].as<std::string>();
        std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;
        msg->GetReflection()->SetString(msg, field, value);
    } break;
    case FieldDescriptor::TYPE_BYTES:
        std::cerr << "Unsupported type BYTES" << std::endl;
        break;
    case FieldDescriptor::TYPE_ENUM:
        std::cerr << "Unsupported type ENUM" << std::endl;
        break;

    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP:
        std::cerr << "Unexpected type!" << std::endl;
    }
}

void Parser::parse(google::protobuf::Message* msg, const std::string& prefix,
                   const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    std::string env_var = prefix + "_" + field->name();
    std::transform(env_var.begin(), env_var.end(), env_var.begin(), [](char c) { return toupper(c); });

    if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
        const Reflection* reflection = msg->GetReflection();

        Message* m = reflection->MutableMessage(msg, field);

        const Descriptor* descriptor = m->GetDescriptor();
        for (int i = 0; i < descriptor->field_count(); i++) {
            const FieldDescriptor* f = descriptor->field(i);

            parse(m, env_var, f);
        }
        return;
    }

    const char* value = std::getenv(env_var.c_str());
    if (value == nullptr) {
        return;
    }

    std::cout << "Setting " << field->name() << " to '" << value << "'" << std::endl;

    switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE:
        msg->GetReflection()->SetDouble(msg, field, std::stod(value));
        break;
    case FieldDescriptor::TYPE_FLOAT:
        msg->GetReflection()->SetFloat(msg, field, std::stof(value));
        break;
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
        msg->GetReflection()->SetInt64(msg, field, std::stoll(value));
        break;
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64:
        msg->GetReflection()->SetUInt64(msg, field, std::stoull(value));
        break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32:
        msg->GetReflection()->SetUInt32(msg, field, std::stoul(value));
        break;
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
        msg->GetReflection()->SetInt32(msg, field, std::stoi(value));
        break;
    case FieldDescriptor::TYPE_BOOL: {
        bool parsed = false;
        std::istringstream is(value);
        is >> std::boolalpha >> parsed;
        msg->GetReflection()->SetBool(msg, field, parsed);
    } break;
    case FieldDescriptor::TYPE_STRING:
        msg->GetReflection()->SetString(msg, field, value);
        break;
    case FieldDescriptor::TYPE_BYTES:
        std::cerr << "Unsupported type BYTES" << std::endl;
        break;
    case FieldDescriptor::TYPE_ENUM:
        std::cerr << "Unsupported type ENUM" << std::endl;
        break;

    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP:
        std::cerr << "Unexpected type!" << std::endl;
    }
}
} // namespace ConfigMuch
