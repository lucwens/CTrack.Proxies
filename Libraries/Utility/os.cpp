#include "os.h"
#include "Print.h"
#include <Windows.h>
#include <stdio.h> // For freopen_s, if you keep it
#include <ios>

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
                // Reopen standard streams
                FILE *dummy;

                // stdout
                freopen_s(&dummy, "CONOUT$", "w", stdout);
                setvbuf(stdout, nullptr, _IONBF, 0); // No buffering

                // stderr
                freopen_s(&dummy, "CONOUT$", "w", stderr);
                setvbuf(stderr, nullptr, _IONBF, 0); // No buffering

                // stdin
                freopen_s(&dummy, "CONIN$", "r", stdin);
                setvbuf(stdin, nullptr, _IONBF, 0); // No buffering

                // Force update of C++ standard streams
                std::ios::sync_with_stdio(true);
            }
            for (int i = 0; i < 10 && hwnd == nullptr; ++i)
            {
                hwnd = GetConsoleWindow();
                if (!hwnd)
                    Sleep(50); // Small delay to give conhost time
            }
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

            // SWP_FRAMECHANGED is essential for the style change to take effect for the taskbar
            // SWP_HIDEWINDOW is redundant here as ShowWindow(SW_HIDE) already hid it, but harmless.
            SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_HIDEWINDOW);

            if (!FreeConsole())
            {
                // FreeConsole can fail if the console was never allocated or is already freed.
                // Handle the error if necessary, but it's not critical for GUI apps.
                DWORD error = GetLastError();
                if (error != ERROR_INVALID_HANDLE && error != ERROR_NOT_SUPPORTED)
                {
                    PrintError("FreeConsole failed with error: {}", error);
                }
            }
        }
    }
}
