#include "os.h"
#include <Windows.h>
#include <stdio.h> // For freopen_s, if you keep it

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


void ShowConsole(bool visible)
{
    HWND hwnd = GetConsoleWindow(); // This gets the HWND of the console attached to your process

    if (visible)
    {
        // If it's a Console Application, hwnd will usually not be NULL here,
        // as the console exists from startup.
        // AllocConsole() and freopen_s are mainly relevant for GUI applications that want a console.
        // For console apps, freopen_s can still be useful if you've done something to redirect.
        if (!hwnd)
        {
            // This block is primarily for GUI apps that dynamically create a console.
            // For a /SUBSYSTEM:CONSOLE app, AllocConsole() typically won't create a *new* console.
            if (AllocConsole())
            {
                FILE *file;
                freopen_s(&file, "CONOUT$", "w", stdout);
                freopen_s(&file, "CONOUT$", "w", stderr);
                freopen_s(&file, "CONIN$", "r", stdin);
            }
            hwnd = GetConsoleWindow(); // Re-fetch after potential allocation
        }

        if (hwnd)
        {
            // Ensure it appears in the taskbar when visible
            long style = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, (style & ~WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW);

            // Apply style changes before showing
            SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

            // Now show the window
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
    }
    else // If visible is false (hide console)
    {
        if (hwnd)
        {
            // First change its style to be a tool window to remove it from the taskbar
            long style = GetWindowLong(hwnd, GWL_EXSTYLE);
            SetWindowLong(hwnd, GWL_EXSTYLE, (style & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);

            // Apply the style change immediately
            SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

            // THEN hide the window
            ShowWindow(hwnd, SW_HIDE);
            FreeConsole();
        }
    }
}