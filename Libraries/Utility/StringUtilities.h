#pragma once

#include <string>
#include <fmt/format.h>

std::wstring StringToWString(const std::string &str);
std::string  WstringToString(const std::wstring &wstr);
std::string  ToUpperCase(const std::string &input);
std::string  ToLowerCase(const std::string &input);
std::string  TrimWhitespaces(const std::string &input);
std::string  ReplaceAll(const std::string &str, const std::string &from, const std::string &to);
void         ParseCommand(const std::string &input, std::string &command, std::vector<std::string> &parameters);
bool         IsNumber(const std::string &str);
