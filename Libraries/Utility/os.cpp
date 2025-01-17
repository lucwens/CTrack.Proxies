#include "os.h"
#include <Windows.h>

void SetThreadName(const std::string &name)
{
    SetThreadDescription(GetCurrentThread(), std::wstring(name.begin(), name.end()).c_str());
}
