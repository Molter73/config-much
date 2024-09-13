#pragma once

#include "internal/parser-env.h"
#include "internal/parser-interface.h"
#include "internal/parser-yaml.h"

#include <google/protobuf/message.h>

#include <optional>

namespace config_much {
class Parser {
public:
    Parser(std::vector<std::unique_ptr<ParserInterface>> parsers) : parsers_(std::move(parsers)) {}

    void parse(google::protobuf::Message* msg) {
        for (auto& parser : parsers_) {
            parser->parse(msg);
        }
    }

private:
    std::vector<std::unique_ptr<ParserInterface>> parsers_;
};

class Builder {
public:
    Builder& add_file(const std::filesystem::path& p) {
        parsers_.emplace_back(std::make_unique<internal::ParserYaml>(p));
        return *this;
    }

    Builder& set_env_var_prefix(const std::string& prefix) {
        env_var_prefix_ = prefix;
        return *this;
    }

    Parser build() {
        if (env_var_prefix_.has_value()) {
            parsers_.emplace_back(std::make_unique<internal::ParserEnv>(*env_var_prefix_));
        }
        return {std::move(parsers_)};
    }

private:
    std::optional<std::string> env_var_prefix_;
    std::vector<std::unique_ptr<ParserInterface>> parsers_;
};
} // namespace config_much
