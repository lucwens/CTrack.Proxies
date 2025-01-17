#pragma once

#include <string>

std::wstring StringToWString(const std::string &str);
std::string WstringToString(const std::wstring &wstr);
std::string ToUpperCase(const std::string &input);
std::string ToLowerCase(const std::string &input);
