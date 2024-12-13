#pragma once

#include "StringUtilities.h"
#include <algorithm>
#include <cctype>
#include <string>

std::string ToUpperCase(const std::string &input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string ToLowerCase(const std::string &input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}


