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

bool IsConsoleVisible()
{
    HWND hConsoleWnd = GetConsoleWindow();
    return (hConsoleWnd != NULL) && IsWindowVisible(hConsoleWnd);
}

void SetConsoleVisible(bool visible)
{
    HWND hwnd = GetConsoleWindow();

    if (!hwnd && visible)
    {
        // Allocate a new console
        if (AllocConsole())
        {
            // Redirect standard streams to the new console
            FILE* file;
            freopen_s(&file, "CONOUT$", "w", stdout);
            freopen_s(&file, "CONOUT$", "w", stderr);
            freopen_s(&file, "CONIN$", "r", stdin);
        }

        hwnd = GetConsoleWindow(); // Re-fetch after allocation
    }

    if (!hwnd)
        return;

    ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);

    // Modify extended window styles to control taskbar visibility
    long style = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, visible ? (style & ~WS_EX_TOOLWINDOW) : (style | WS_EX_TOOLWINDOW));

    if (visible)
    {
        SetForegroundWindow(hwnd);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }
}