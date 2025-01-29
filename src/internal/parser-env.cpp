#include "internal/parser-env.h"

#include <cstdlib>
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

    if (field->label() == FieldDescriptor::LABEL_REPEATED) {
        return parse_array(msg, env_var, field);
    }

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
    case FieldDescriptor::TYPE_ENUM: {
        const EnumValueDescriptor* v = field->enum_type()->FindValueByName(value);
        msg->GetReflection()->SetEnum(msg, field, v);
    } break;

    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP:
        std::cerr << "Unexpected type!" << std::endl;
    }
}

namespace {
template <typename T>
void parse_array_inner(google::protobuf::MutableRepeatedFieldRef<T> field, const std::string& prefix,
                       std::function<T(const char*)> p) {
    std::string name;
    const char* value = nullptr;
    field.Clear();
    for (int i = 0;; i++) {
        name  = prefix + '_' + std::to_string(i);
        value = std::getenv(name.c_str());
        if (value == nullptr) {
            break;
        }

        field.Add(p(value));
    }
}
} // namespace

void ParserEnv::parse_array(google::protobuf::Message* msg, const std::string& prefix,
                            const google::protobuf::FieldDescriptor* field) {
    using namespace google::protobuf;
    switch (field->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<int32_t>(msg, field);
        parse_array_inner<int32_t>(f, prefix, [](const char* s) { return std::stoi(s); });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_INT64: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<int64_t>(msg, field);
        parse_array_inner<int64_t>(f, prefix, [](const char* s) { return std::stoi(s); });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<uint32_t>(msg, field);
        parse_array_inner<uint32_t>(f, prefix, [](const char* s) { return std::stoul(s); });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<uint64_t>(msg, field);
        parse_array_inner<uint64_t>(f, prefix, [](const char* s) { return std::stoull(s); });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<double>(msg, field);
        parse_array_inner<double>(f, prefix, [](const char* s) { return std::stod(s); });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<float>(msg, field);
        parse_array_inner<float>(f, prefix, [](const char* s) { return std::stof(s); });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<bool>(msg, field);
        parse_array_inner<bool>(f, prefix, [](const char* s) {
            bool parsed = false;
            std::istringstream is(s);
            is >> std::boolalpha >> parsed;
            return parsed;
        });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<int32>(msg, field);
        parse_array_inner<int32>(f, prefix, [&](const char* s) {
            const google::protobuf::EnumValueDescriptor* v = field->enum_type()->FindValueByName(s);
            return v->number();
        });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        auto f = msg->GetReflection()->GetMutableRepeatedFieldRef<std::string>(msg, field);
        parse_array_inner<std::string>(f, prefix, [](const char* s) { return s; });
    } break;
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
        std::cerr << "Unsupported repeated type MESSAGE" << std::endl;
        break;
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
