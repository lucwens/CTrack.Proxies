#pragma once

#include <iostream>
#include <string>

#include "Print.h"

std::mutex printMutex;

std::string FormatString(const std::string format, ...)
{
    va_list args;
    va_start(args, format);

    // Estimate required size
    int size = std::vsnprintf(nullptr, 0, format.c_str(), args) + 1; // Include space for null terminator
    va_end(args);

    if (size <= 0)
    {
        return ""; // Formatting error
    }

    std::string result(size, '\0');

    va_start(args, format);
    std::vsnprintf(&result[0], size, format.c_str(), args);
    va_end(args);

    result.pop_back(); // Remove null terminator
    return result;
}

void SetConsoleColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
