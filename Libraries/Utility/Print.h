#pragma once

#include <fmt/format.h>
#include <iostream>
#include <mutex>
#include <string>
#include <windows.h>

// Global mutex for thread-safe printing
extern std::mutex printMutex;

const int RED     = 1;
const int GREEN   = 2;
const int YELLOW  = 3;
const int BLUE    = 4;
const int MAGENTA = 5;
const int CYAN    = 6;
const int WHITE   = 7;
const int BLACK   = 0;

// Helper function to set console text color
void SetConsoleBackgroundColor(WORD color);
void SetConsoleTextColor(WORD color);
void DebugPrintShowHideConsole(bool bShow);
void SetConsoleTabText(const char *newTitle);
void SetConsoleTabBackgroundColor(int Color);
void EnableAnsiColors();
void PrintTimeStamp();

template <typename... Args> void PrintColor(int r, int g, int b, const std::string &format, Args &&...args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    EnableAnsiColors(); // make sure ANSI sequences are supported
    fmt::print("\x1b[38;2;{};{};{}m", r, g, b);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);
    fmt::print("\x1b[0m"); // reset to default
}

// PrintInfo function
template <typename... Args> void Print(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    fmt::print(format + "\n", std::forward<Args>(args)...); // fmt handles formatting
}

// PrintInfo function
template <typename... Args> void PrintInfo(const std::string &format, Args &&...args)
{
    PrintColor(0, 255, 0, format, std::forward<Args>(args)...);
}

// PrintInfo function
template <typename... Args> void PrintMessage(const std::string &format, Args &&...args)
{
    PrintColor(125, 125, 125, format, std::forward<Args>(args)...);
}

// PrintWarning function
template <typename... Args> void PrintWarning(const std::string &format, Args... args)
{
    PrintColor(255, 165, 0, format, std::forward<Args>(args)...);
}

// PrintError function
template <typename... Args> void PrintError(const std::string &format, Args... args)
{
    PrintColor(255, 0, 0, format, std::forward<Args>(args)...);
}

// PrintCommand function
template <typename... Args> void PrintCommand(const std::string &format, Args... args)
{
    PrintColor(100, 100, 255, format, std::forward<Args>(args)...);
}

// PrintCommand function
template <typename... Args> void PrintCommandReturn(const std::string &format, Args... args)
{
    PrintColor(200, 200, 255, format, std::forward<Args>(args)...);
}
