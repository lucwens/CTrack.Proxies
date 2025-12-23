
#include "Print.h"
#include "Logging.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>

std::mutex printMutex;

void EnableAnsiColors()
{
    static bool initialized = false;
    if (!initialized)
    {
        HANDLE hOut   = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD  dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
            initialized = true;
        }
    }
}

void SetConsoleBackgroundColor(WORD color)
{
    //     HANDLE                     hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    //     CONSOLE_SCREEN_BUFFER_INFO csbi;
    //     GetConsoleScreenBufferInfo(hConsole, &csbi);
    //     WORD newColor = (csbi.wAttributes & 0x0F) | (color << 4);
    //     SetConsoleTextAttribute(hConsole, newColor);
}

void SetConsoleTextColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void SetConsoleTabText(const char *newTitle)
{
    std::cout << "\033]0;" << newTitle << "\007";
    std::cout.flush(); // Ensure the sequence is sent immediately
}

void SetConsoleTabBackgroundColor(int Color)
{
    //
    std::string Code = fmt::format("\033[2;15;{},|", Color);
    std::cout << Code;
    std::cout.flush();
}

void DebugPrintShowHideConsole(bool bShow)
{
    HWND hWnd = GetConsoleWindow();
    if (hWnd)
    {
        ShowWindow(hWnd, bShow ? SW_SHOWNORMAL : SW_HIDE);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        ShowWindow(hWnd, bShow ? SW_SHOWNORMAL : SW_HIDE);
    }
}

void PrintTimeStamp()
{
    std::cout << CTrack::GetTimeStampString() << " ";
}

void PrintStatusTopRight(const std::string &status)
{
    std::lock_guard<std::mutex> lock(printMutex);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        return;

    // Get current cursor position and console info
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return;

    // Save current cursor position
    COORD savedPos   = csbi.dwCursorPosition;

    // Calculate position for top-right corner
    // Leave some padding from the right edge
    int statusLength = static_cast<int>(status.length());
    int xPos         = csbi.dwSize.X - statusLength - 1;
    if (xPos < 0)
        xPos = 0;

    // Move cursor to top-right (row 0)
    COORD topRight = {static_cast<SHORT>(xPos), 0};
    SetConsoleCursorPosition(hConsole, topRight);

    // Print with cyan color using ANSI codes
    EnableAnsiColors();
    fmt::print("\x1b[38;2;0;255;255m{}\x1b[0m", status);

    // Restore cursor position
    SetConsoleCursorPosition(hConsole, savedPos);

    // Flush to ensure immediate display
    std::cout.flush();
}
