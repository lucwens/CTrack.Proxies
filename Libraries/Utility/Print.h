#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <windows.h>

// Global mutex for thread-safe printing
extern std::mutex printMutex;

// Helper function to set console text color
void SetConsoleColor(WORD color);

std::string FormatString(const std::string format, ...);

// PrintInfo function
template <typename... Args> void Print(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::printf(format.c_str(), args...);
    std::printf("\n");                                                    // Add a newline explicitly if needed
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintInfo function
template <typename... Args> void PrintInfo(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::printf(format.c_str(), args...);
    std::printf("\n");                                                    // Add a newline explicitly if needed
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintWarning function
template <typename... Args> void PrintWarning(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::printf(format.c_str(), args...);
    std::printf("\n");                                                    // Add a newline explicitly if needed
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintError function
template <typename... Args> void PrintError(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::printf(format.c_str(), args...);
    std::printf("\n");                                                    // Add a newline explicitly if needed
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}

// PrintCommand function
template <typename... Args> void PrintCommand(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::printf(format.c_str(), args...);
    std::printf("\n");                                                    // Add a newline explicitly if needed
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
}
