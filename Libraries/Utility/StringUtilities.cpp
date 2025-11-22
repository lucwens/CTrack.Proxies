#pragma once

#include "StringUtilities.h"

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <sstream>
#include <string>
#include <Windows.h>

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

std::wstring StringToWString(const std::string &str)
{
    std::wstring returnString;
    // On Windows, use MultiByteToWideChar
    int          sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    try
    {
        returnString.resize(sizeNeeded);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &returnString[0], sizeNeeded);
    }
    catch (...)
    {
        returnString = L"Error message conversion error";
    }
    return returnString;
}

std::string WstringToString(const std::wstring &wstr)
{
    std::string returnString;
    try
    {
        int         sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string str(sizeNeeded, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &returnString[0], sizeNeeded, NULL, NULL);
    }
    catch (...)
    {
        returnString = "Error message conversion error";
    }
    return returnString;
}

std::string TrimWhitespaces(const std::string &s)
{
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); });

    auto end   = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();

    if (start >= end)
        return ""; // alleen witruimte

    return std::string(start, end);
}

std::string ReplaceAll(const std::string &str, const std::string &from, const std::string &to)
{
    std::string result    = str;
    size_t      start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos)
    {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return result;
}

std::string trim(const std::string &s)
{
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); });

    auto end   = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();

    return (start < end) ? std::string(start, end) : "";
}

void ParseCommand(const std::string &input, std::string &command, std::vector<std::string> &parameters)
{
    parameters.clear(); // reset lijst

    size_t openParen  = input.find('(');
    size_t closeParen = input.rfind(')');

    if (openParen == std::string::npos || closeParen == std::string::npos || openParen >= closeParen)
    {
        command = trim(input);
        return;
    }

    command                 = trim(input.substr(0, openParen));
    std::string paramString = input.substr(openParen + 1, closeParen - openParen - 1);

    std::stringstream ss(paramString);
    std::string       token;

    while (std::getline(ss, token, ','))
    {
        parameters.push_back(trim(token));
    }
}

bool IsNumber(const std::string &str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

bool CompareIgnoreCase(const std::string &a, const std::string &b)
{
    if (a.size() != b.size())
        return false;
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}
