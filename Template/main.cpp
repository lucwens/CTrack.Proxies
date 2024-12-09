// Template.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../Libraries/Utility/Print.h"
#include "../Libraries/Utility/errorHandling.h"
#include <iostream>

int main()
{
    try
    {
        Print("Hello world");
        PrintInfo("Hello world with info");
        PrintWarning("Something wrong");
        THROW_ERROR("This is an error message");
    }
    catch (const std::exception& e)
    {
        PrintError(e.what());
    }
    catch (...)
    {
        PrintError("An unknown error occurred");
    }
}
