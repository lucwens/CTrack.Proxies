#pragma once

#include <fmt/format.h>
#include <iostream>
#include <mutex>
#include <string>
#include <windows.h>

// Global mutex for thread-safe printing
extern std::mutex printMutex;

// Helper function to set console text color
void SetConsoleBackgroundColor(WORD color);
void SetConsoleTextColor(WORD color);
void DebugPrintShowHideConsole(bool bShow);

void        PrintTimeStamp();

// PrintInfo function
template <typename... Args> void Print(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);               // fmt handles formatting
}

// PrintInfo function
template <typename... Args> void PrintInfo(const std::string &format, Args &&...args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);               // fmt handles formatting
    SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintWarning function
template <typename... Args> void PrintWarning(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);               // fmt handles formatting
    SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintError function
template <typename... Args> void PrintError(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);               // fmt handles formatting
    SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintCommand function
template <typename... Args> void PrintCommand(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleTextColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);               // fmt handles formatting
    SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}
