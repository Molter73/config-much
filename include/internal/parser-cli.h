#pragma once

#include "internal/parser-interface.h"

#include <getopt.h>

namespace config_much::internal {
class ParserCli : public ParserInterface {
public:
    void parse(google::protobuf::Message* msg) override;
};

enum HasArgument {
    NoArgument = no_argument,
    RequiredArgument = required_argument,
    OptionalArgument = optional_argument,
};

template <typename T>
class CliArg {
public:
private:
    char short_opt_;
    HasArgument has_argument_;
    std::string long_opt_;
    std::string description_;
    std::function<T (const char*)> parser_;
};
} // namespace config_much::internal
