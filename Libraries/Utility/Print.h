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

// Message type enumeration for colored console output
enum class MessageType
{
    Info,           // Green - informational messages
    Message,        // Gray - general messages
    Warning,        // Orange - warnings
    Error,          // Red - errors
    Command,        // Blue - commands sent
    CommandReturn,  // Light Blue - command responses
    Debug           // Magenta - debug information
};

// Color lookup structure
struct MessageColor
{
    int r;
    int g;
    int b;
};

// Get RGB color for a message type
inline MessageColor GetMessageColor(MessageType type)
{
    switch (type)
    {
        case MessageType::Info:
            return {0, 255, 0}; // Green
        case MessageType::Message:
            return {125, 125, 125}; // Gray
        case MessageType::Warning:
            return {255, 165, 0}; // Orange
        case MessageType::Error:
            return {255, 0, 0}; // Red
        case MessageType::Command:
            return {100, 100, 255}; // Blue
        case MessageType::CommandReturn:
            return {200, 200, 255}; // Light Blue
        case MessageType::Debug:
            return {255, 100, 255}; // Magenta
        default:
            return {255, 255, 255}; // White (fallback)
    }
}

// Helper function to set console text color
void SetConsoleBackgroundColor(WORD color);
void SetConsoleTextColor(WORD color);
void DebugPrintShowHideConsole(bool bShow);
void SetConsoleTabText(const char *newTitle);
void SetConsoleTabBackgroundColor(int Color);
void EnableAnsiColors();
void PrintTimeStamp();
void PrintStatusTopRight(const std::string &status);

// New enum-based PrintColor functions
template <typename... Args> void PrintColor(MessageType type, const std::string &format, Args &&...args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    EnableAnsiColors(); // make sure ANSI sequences are supported
    MessageColor color = GetMessageColor(type);
    fmt::print("\x1b[38;2;{};{};{}m", color.r, color.g, color.b);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);
    fmt::print("\x1b[0m"); // reset to default
}

inline void PrintColor(MessageType type, const std::string &text)
{
    std::lock_guard<std::mutex> lock(printMutex);
    EnableAnsiColors(); // make sure ANSI sequences are supported
    MessageColor color = GetMessageColor(type);
    fmt::print("\x1b[38;2;{};{};{}m", color.r, color.g, color.b);
    PrintTimeStamp();
    std::cout << text << std::endl;
    fmt::print("\x1b[0m"); // reset to default
}

// Legacy RGB-based PrintColor functions (for backward compatibility)
template <typename... Args> void PrintColorRGB(int r, int g, int b, const std::string &format, Args &&...args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    EnableAnsiColors(); // make sure ANSI sequences are supported
    fmt::print("\x1b[38;2;{};{};{}m", r, g, b);
    PrintTimeStamp();
    fmt::print(format + "\n", std::forward<Args>(args)...);
    fmt::print("\x1b[0m"); // reset to default
}

inline void PrintColorRGB(int r, int g, int b, const std::string &text)
{
    std::lock_guard<std::mutex> lock(printMutex);
    EnableAnsiColors(); // make sure ANSI sequences are supported
    fmt::print("\x1b[38;2;{};{};{}m", r, g, b);
    PrintTimeStamp();
    std::cout << text << std::endl;
    fmt::print("\x1b[0m"); // reset to default
}

// PrintInfo function
template <typename... Args> void Print(const std::string &format, Args... args)
{
    std::lock_guard<std::mutex> lock(printMutex);
    fmt::print(format + "\n", std::forward<Args>(args)...); // fmt handles formatting
}

// PrintInfo function - informational messages (green)
template <typename... Args> void PrintInfo(const std::string &format, Args &&...args)
{
    PrintColor(MessageType::Info, format, std::forward<Args>(args)...);
}

// PrintMessage function - general messages (gray)
template <typename... Args> void PrintMessage(const std::string &format, Args &&...args)
{
    PrintColor(MessageType::Message, format, std::forward<Args>(args)...);
}

// PrintWarning function - warnings (orange)
template <typename... Args> void PrintWarning(const std::string &format, Args... args)
{
    PrintColor(MessageType::Warning, format, std::forward<Args>(args)...);
}

// PrintError function - errors (red)
template <typename... Args> void PrintError(const std::string &format, Args... args)
{
    PrintColor(MessageType::Error, format, std::forward<Args>(args)...);
}

// PrintCommand function - commands sent (blue)
template <typename... Args> void PrintCommand(const std::string &format, Args... args)
{
    PrintColor(MessageType::Command, format, std::forward<Args>(args)...);
}

// PrintCommandReturn function - command responses (light blue)
template <typename... Args> void PrintCommandReturn(const std::string &format, Args... args)
{
    PrintColor(MessageType::CommandReturn, format, std::forward<Args>(args)...);
}

// PrintDebug function - debug information (magenta)
template <typename... Args> void PrintDebug(const std::string &format, Args... args)
{
    PrintColor(MessageType::Debug, format, std::forward<Args>(args)...);
}
