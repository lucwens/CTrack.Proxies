#pragma once

#include <string>
#include <vector>
#include <fmt/format.h>

std::wstring StringToWString(const std::string &str);
std::string  WstringToString(const std::wstring &wstr);
std::string  ToUpperCase(const std::string &input);
std::string  ToLowerCase(const std::string &input);
std::string  TrimWhitespaces(const std::string &input);
std::string  ReplaceAll(const std::string &str, const std::string &from, const std::string &to);
void         ParseCommand(const std::string &input, std::string &command, std::vector<std::string> &parameters);
bool         IsNumber(const std::string &str);
bool         CompareIgnoreCase(const std::string &str1, const std::string &str2);

#ifdef _MANAGED

#include <string>
#include <msclr/marshal_cppstd.h>

inline std::string ToStdString(System::String ^ managedStr)
{
    return msclr::interop::marshal_as<std::string>(managedStr);
}
#endif
