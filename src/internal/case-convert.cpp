#include "internal/case-convert.h"
#include <algorithm>

namespace config_much::case_convert {
std::string all_caps(const std::string& input) {
    std::string out;
    out.resize(input.length());
    std::transform(input.begin(), input.end(), out.begin(), [](char c) { return std::toupper(c); });
    return out;
}

std::string snake_to_camel(const std::string& input) {
    std::string out;
    bool capitalize = false;

    for (const auto& c : input) {
        if (c == '_') {
            capitalize = true;
        } else {
            out += capitalize ? (char)std::toupper(c) : c;
            capitalize = false;
        }
    }

    return out;
}

std::string camel_to_snake(const std::string& input, bool capitalize) {
    std::string out;
    bool first = true;

    for (const auto& c : input) {
        if (!first && std::isupper(c) != 0) {
            out += '_';
        }
        out += capitalize ? (char)std::toupper(c) : (char)std::tolower(c);
        first = false;
    }

    return out;
}
} // namespace config_much::case_convert
