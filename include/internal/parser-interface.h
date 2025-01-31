#pragma once

#include "parser-error.h"

#include <google/protobuf/message.h>

namespace config_much {

class ParserInterface {
public:
    virtual ~ParserInterface()                         = default;
    ParserInterface(const ParserInterface&)            = default;
    ParserInterface(ParserInterface&&)                 = default;
    ParserInterface& operator=(const ParserInterface&) = default;
    ParserInterface& operator=(ParserInterface&&)      = default;
    ParserInterface()                                  = default;

    virtual ParserResult parse(google::protobuf::Message* msg) = 0;
};
} // namespace config_much
