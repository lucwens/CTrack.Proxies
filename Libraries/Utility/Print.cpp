
#include "Print.h"
#include "Logging.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>

std::mutex printMutex;

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
    std::cout << GetTimeStampString() << " ";
}
