#pragma once

#include <optional>
#include <ostream>
#include <sstream>
#include <vector>

namespace config_much {

class ParserError {
public:
    ParserError()                                  = default;
    ParserError(ParserError&&) noexcept            = default;
    ParserError(const ParserError&)                = default;
    ParserError& operator=(const ParserError&)     = default;
    ParserError& operator=(ParserError&&) noexcept = default;
    ~ParserError()                                 = default;

    ParserError(const char* msg) { msg_ += msg; }
    ParserError(const std::string& msg) { msg_ += msg; }

    const std::string& what() const { return msg_; }

    template <typename T> friend ParserError& operator<<(ParserError& e, const T msg) {
        std::stringstream ss;
        ss << msg;
        e.msg_ += ss.str();
        return e;
    }

    friend std::ostream& operator<<(std::ostream& os, const ParserError& err) {
        os << err.what();
        return os;
    }

    friend bool operator==(const ParserError& lhs, const ParserError& rhs) { return lhs.msg_ == rhs.msg_; }

private:
    std::string msg_;
};

using ParserErrors = std::vector<ParserError>;
using ParserResult = std::optional<ParserErrors>;

} // namespace config_much
