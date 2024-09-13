#pragma once

#include <google/protobuf/message.h>

namespace config_much {
class ParserInterface {
public:
    virtual ~ParserInterface()                         = default;
    ParserInterface(const ParserInterface&)            = default;
    ParserInterface(ParserInterface&&)                 = default;
    ParserInterface& operator=(const ParserInterface&) = default;
    ParserInterface& operator=(ParserInterface&&)      = default;
    ParserInterface() = default;

    virtual void parse(google::protobuf::Message* msg) = 0;
};
} // namespace config_much
