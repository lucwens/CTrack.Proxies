#pragma once

#include <string>
#include <locale>
#include <codecvt>

// Convert std::string to std::wstring
std::wstring StringToWString(const std::string &str);

// Convert std::wstring to std::string
std::string WstringToString(const std::wstring &wstr);

void SetThreadName(const std::string &name);
