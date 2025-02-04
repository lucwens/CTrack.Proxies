#pragma once

#include <vector>
#include <string>

class FileReader
{
  public:
    static std::vector<std::vector<double>> ReadNumbersFromFile(const std::string &filename);
};
