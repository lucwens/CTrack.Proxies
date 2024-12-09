#include "os.h"
#include <locale>
#include <codecvt>
#include <Windows.h>

// Convert std::string to std::wstring
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

// Convert std::wstring to std::string
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

void SetThreadName(const std::string &name)
{
    SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
}
