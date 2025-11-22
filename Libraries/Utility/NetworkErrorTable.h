#pragma once

#include <string>
#include <map>
#include <winsock2.h>

struct WSA_ERROR_TABLE
{
    std::string ErrorString;
    std::string ShortDescription;
    std::string LongDescription;
};

extern std::map<int, WSA_ERROR_TABLE> NetworkErrorMap;
