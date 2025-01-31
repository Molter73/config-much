#pragma once

#include "internal/parser-env.h"
#include "internal/parser-error.h"
#include "internal/parser-interface.h"
#include "internal/parser-yaml.h"

#include <google/protobuf/message.h>

#include <optional>

namespace config_much {
class Parser {
public:
    Parser() = default;

    ParserResult parse(google::protobuf::Message* msg) {
        ParserErrors errors;

        for (auto& parser : parsers_) {
            auto err = parser->parse(msg);
            if (err) {
                errors.insert(errors.end(), err->begin(), err->end());
            }
        }

        if (parser_env_) {
            auto err = parser_env_->parse(msg);
            if (err) {
                errors.insert(errors.end(), err->begin(), err->end());
            }
        }

        if (!errors.empty()) {
            return errors;
        }
        return {};
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
