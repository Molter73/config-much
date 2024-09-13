#include "internal/parser-env.h"

#include <google/protobuf/descriptor.h>

namespace config_much::internal {
void ParserEnv::parse(google::protobuf::Message* msg) {
    using namespace google::protobuf;

    const Descriptor* descriptor = msg->GetDescriptor();
    for (int i = 0; i < descriptor->field_count(); i++) {
        const FieldDescriptor* field = descriptor->field(i);
        parse(msg, prefix_, field);
    }
}

void ParserEnv::parse(google::protobuf::Message* msg, const std::string& prefix,
                      const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;

    std::string env_var = cook_env_var(prefix, field->name());

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

std::string ParserEnv::cook_env_var(const std::string& prefix, const std::string& suffix) {
    return prefix + '_' + to_upper(camel_to_snake_case(suffix));
}

std::string ParserEnv::camel_to_snake_case(const std::string& s) {
    std::string out;
    bool first = true;

    for (const auto& c : s) {
        if (!first && std::isupper(c) != 0) {
            out += '_';
        }
        out += (char)std::tolower(c);
        first = false;
    }

    return out;
}

std::string ParserEnv::to_upper(const std::string& s) {
    std::string out;
    out.resize(s.size());
    std::transform(s.begin(), s.end(), out.begin(), [](unsigned char c) { return std::toupper(c); });
    return out;
}
} // namespace config_much::internal
