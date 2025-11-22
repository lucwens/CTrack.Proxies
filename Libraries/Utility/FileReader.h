#pragma once

#include <vector>
#include <string>

namespace FileReader
{
    std::vector<std::vector<double>> ReadNumbersFromFile(const std::string &filename, const std::string &delimiters = ";|,\\s");
};
