#include "os.h"
#include <Windows.h>

void ThreadSetName(const std::string &name)
{
    SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
}

void ThreadSetName(std::thread &rThread, const std::string &name)
{
    SetThreadDescription(rThread.native_handle(), std::wstring(name.begin(), name.end()).c_str());
}
