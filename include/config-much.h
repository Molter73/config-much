#pragma once

#include "internal/parser-env.h"
#include "internal/parser-interface.h"
#include "internal/parser-yaml.h"

#include <google/protobuf/message.h>

#include <optional>

namespace config_much {
class Parser {
public:
    Parser() = default;

    void parse(google::protobuf::Message* msg) {
        for (auto& parser : parsers_) {
            parser->parse(msg);
        }

        if (parser_env_) {
            parser_env_->parse(msg);
        }
    }

    Parser& add_file(const std::filesystem::path& path) {
        parsers_.emplace_back(std::make_unique<internal::ParserYaml>(path));
        return *this;
    }

    Parser& set_env_var_prefix(const std::string& prefix) {
        parser_env_ = internal::ParserEnv(prefix);
        return *this;
    }

private:
    std::vector<std::unique_ptr<ParserInterface>> parsers_;
    std::optional<internal::ParserEnv> parser_env_;
};
} // namespace config_much
