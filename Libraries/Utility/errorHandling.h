#pragma once

#include "os.h"
#include "print.h"

#include <string>
#include <exception>
#include <sstream>

class DetailedException : public std::exception
{
  public:
    DetailedException(const std::string &message, const char *file, int line) : msg_(FormatMessage(message, file, line))
    {
    }

    const char *what() const noexcept override
    {
        return msg_.c_str();
    }

  private:
    std::string msg_;

    static std::string FormatMessage(const std::string &message, const char *file, int line)
    {
        std::ostringstream oss;
        oss << message << " (File: " << file << ", Line: " << line << ")";
        return oss.str();
    }
};

class DetailedSocketException : public DetailedException
{
  public:
    DetailedSocketException(const std::string &message, int NetworkError ,const char *file, int line) : DetailedException(message, file, line)
    {
        //workError_ = NetworkError;
    }
};

#define THROW_DETAILED_EXCEPTION(msg) throw DetailedException((msg), __FILE__, __LINE__)


#define THROW_ERROR(a)                throw DetailedException(a, __FILE__, __LINE__)
#define THROW_SOCKET_ERROR(a, b)      throw DetailedSocketException(a,b, __FILE__, __LINE__)