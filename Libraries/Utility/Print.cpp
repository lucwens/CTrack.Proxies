#pragma once

#include "Print.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>


std::mutex printMutex;
auto       startFromProgram = std::chrono::system_clock::now();

void SetConsoleColor(WORD color)
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

std::string GetDateTimeString()
{
    // Get the current time
    auto   now                 = std::chrono::system_clock::now();
    auto   now_time_t          = std::chrono::system_clock::to_time_t(now);
    auto   now_ms              = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto   duration            = std::chrono::duration_cast<std::chrono::milliseconds>(now - startFromProgram).count();
    double duration_in_seconds = duration / 1000.0;

    // Format the time with three decimal places
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << now_ms.count() << "[" << duration_in_seconds << "]" << " : ";
    return oss.str();
}

void PrintTimeStamp()
{
    std::cout << GetDateTimeString();
}
