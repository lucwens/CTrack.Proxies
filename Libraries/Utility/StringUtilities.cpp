#pragma once

#include "StringUtilities.h"

#include <algorithm>
#include <cctype>
#include <codecvt>
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
