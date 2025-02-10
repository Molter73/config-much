#pragma once

#include <string>

namespace config_much::case_convert {

std::string all_caps(const std::string& input);
std::string snake_to_camel(const std::string& input);
std::string camel_to_snake(const std::string& input, bool capitalize = false);

} // namespace config_much::case_convert
