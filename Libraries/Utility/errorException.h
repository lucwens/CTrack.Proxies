#pragma once

#include "NetworkError.h"
#include <string>
#include <exception>
#include <sstream>

class CDetailedException : public std::exception
{
  public:
    CDetailedException(const std::string &message, const char *file, int line) : msg(FormatMessage(message, file, line)) {}

    const char              *what() const noexcept override { return msg.c_str(); }
    
  protected:
    std::string              msg;

    static std::string FormatMessage(const std::string &message, const char *file, int line)
    {
        std::ostringstream oss;
        oss << message << " (File: " << file << ", Line: " << line << ")";
        return oss.str();
    }
};

class CDetailedSocketException : public CDetailedException
{
  public:
    CDetailedSocketException(const std::string &message, int NetworkError, const char *file, int line) : CDetailedException(message, file, line)
    {
        std::string NetworkErrorMessage = XWSA_GetLongDescription(NetworkError);
        msg += " (Network Error: " + NetworkErrorMessage + ")";
    }
};

#define THROW_ERROR(msg)                   throw CDetailedException((msg), __FILE__, __LINE__)
#define THROW_SOCKET_ERROR(msg, errorcode) throw CDetailedSocketException((msg), errorcode, __FILE__, __LINE__)
