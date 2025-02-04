#include "FileReader.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

std::vector<std::vector<double>> FileReader::ReadNumbersFromFile(const std::string &filename)
{
    std::vector<std::vector<double>> data;
    std::ifstream                    file(filename);

    if (!file)
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return data;
    }

    std::string line;
    std::regex  delimiter("[;|,\\s]+"); // Match semicolon, pipe, comma, or whitespace

    while (std::getline(file, line))
    {
        std::vector<double>        row;
        std::sregex_token_iterator it(line.begin(), line.end(), delimiter, -1);
        std::sregex_token_iterator end;

        for (; it != end; ++it)
        {
            std::string value = it->str();
            if (!value.empty()) // Ignore empty values
            {
                try
                {
                    row.push_back(std::stod(value)); // Convert to double
                }
                catch (const std::invalid_argument &)
                {
                    std::cerr << "Invalid number found in file: " << value << std::endl;
                }
                catch (const std::out_of_range &)
                {
                    std::cerr << "Number out of range in file: " << value << std::endl;
                }
            }
        }

        if (!row.empty())
        { // Avoid empty lines
            data.push_back(row);
        }
    }

    return data;
}