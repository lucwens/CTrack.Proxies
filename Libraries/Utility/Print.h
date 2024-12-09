#pragma once

#include <iostream>
#include <string>
#include <format>
#include <windows.h>
#include <mutex>

// Global mutex for thread-safe printing
extern std::mutex printMutex;

// Helper function to set console text color
void SetConsoleColor(WORD color);

// PrintInfo function
template <typename... Args> void Print(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::vformat(format, std::make_format_args(args...)) << std::endl;
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintInfo function
template <typename... Args> void PrintInfo(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << std::vformat(format, std::make_format_args(args...)) << std::endl;
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintWarning function
template <typename... Args> void PrintWarning(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << std::vformat(format, std::make_format_args(args...)) << std::endl;
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintError function
template <typename... Args> void PrintError(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << std::vformat(format, std::make_format_args(args...)) << std::endl;
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}
