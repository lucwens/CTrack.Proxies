#include "os.h"
#include <locale>
#include <codecvt>

#ifdef _WIN32
// Windows-specific code
  #ifndef _WINDOWS_
    #include <Windows.h>
  #endif
// Use Windows-specific functions like WideCharToMultiByte :Standard library header <codecvt>
// (C++11)(deprecated in C++17)(removed in C++26) here a windows specific code is used to convert wide string to
// utf8 string, a linux version would be needed
#else
  // Linux-specific code
  #include <locale>
  #include <codecvt>
// Use Linux-compatible functions or standard C++ conversions
#endif

namespace xenomatix::sdk
{

  // Convert std::string to std::wstring
  std::wstring StringToWString(const std::string& str)
  {
    std::wstring returnString;
#ifdef _WIN32
    // On Windows, use MultiByteToWideChar
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.size(), NULL, 0);
    try
    {
      returnString.resize(sizeNeeded);
      MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.size(), &returnString[0], sizeNeeded);
    }
    catch (...)
    {
      returnString = L"Error message conversion error";
    }
#else
    // On Linux, use std::wstring_convert
    try
    {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      returnString = converter.from_bytes(str);
    }
    catch (...)
    {
      returnString = L"Error message conversion error";
    }
#endif
    return returnString;
  }

  // Convert std::wstring to std::string
  std::string WstringToString(const std::wstring& wstr)
  {
    std::string returnString;
#ifdef _WIN32
    // On Windows, use WideCharToMultiByte
    try
    {
      int         sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int) wstr.size(), NULL, 0, NULL, NULL);
      std::string str(sizeNeeded, 0);
      WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int) wstr.size(), &returnString[0], sizeNeeded, NULL, NULL);
    }
    catch (...)
    {
      returnString = "Error message conversion error";
    }
#else
    // On Linux, use std::wstring_convert
    try
    {
      std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      returnString = converter.to_bytes(wstr);
    }
    catch (...)
    {
      returnString = "Error message conversion error";
    }
#endif
    return returnString;
  }

  void SetThreadName(const std::string& name)
  {
#if defined(_WIN32) || defined(_WIN64)
    // Windows specific: SetThreadDescription API (Windows 10, version 1607 and later)
    HRESULT hr = SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
    if (FAILED(hr))
    {
      //std::cerr << "Failed to set thread name on Windows: " << hr << std::endl;
    }
#elif defined(__linux__)
    // Linux specific: pthread_setname_np (limited to 15 characters)
    if (pthread_setname_np(pthread_self(), name.substr(0, 15).c_str()) != 0)
    {
      //std::cerr << "Failed to set thread name on Linux" << std::endl;
    }
#else
    // Unsupported platform
    //std::cerr << "Setting thread name is not supported on this platform." << std::endl;
#endif
  }

} // namespace xenomatix::sdk
