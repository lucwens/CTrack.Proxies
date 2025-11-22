
#include "NetworkErrorTable.h"

#include <winsock2.h>
#include <ws2tcpip.h>

std::string XWSA_GetErrorString(int nErrorCode)
{
    char buffer[101];
    sprintf_s(buffer, sizeof(buffer) / sizeof(char), "Unknown network error %d", nErrorCode);

    std::string strErrorString(buffer);

    if (NetworkErrorMap.find(nErrorCode) != NetworkErrorMap.end())
    {
        strErrorString = NetworkErrorMap[nErrorCode].ErrorString;
    }
    return strErrorString;
}

std::string XWSA_GetShortDescription(int nErrorCode)
{
    char buffer[101];
    sprintf_s(buffer, sizeof(buffer) / sizeof(char), "No short description for network error %d", nErrorCode);

    std::string strErrorString(buffer);

    if (NetworkErrorMap.find(nErrorCode) != NetworkErrorMap.end())
    {
        strErrorString = NetworkErrorMap[nErrorCode].ShortDescription;
    }
    return strErrorString;
}

std::string XWSA_GetLongDescription(int nErrorCode)
{
    char buffer[101];
    sprintf_s(buffer, sizeof(buffer) / sizeof(char), "No long description for network error %d", nErrorCode);

    std::string strErrorString(buffer);

    if (NetworkErrorMap.find(nErrorCode) != NetworkErrorMap.end())
    {
        strErrorString = NetworkErrorMap[nErrorCode].LongDescription;
    }
    return strErrorString;
}
