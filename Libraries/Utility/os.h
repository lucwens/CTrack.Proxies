#pragma once

#include <string>
#include <thread>

// set the name of the current thread
void ThreadSetName(const std::string &name);
void ThreadSetName(std::thread &rThread, const std::string &name);
