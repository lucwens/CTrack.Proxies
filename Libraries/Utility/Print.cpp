#pragma once

#include <iostream>
#include <string>

#include "Print.h"

std::mutex printMutex;

void SetConsoleColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
