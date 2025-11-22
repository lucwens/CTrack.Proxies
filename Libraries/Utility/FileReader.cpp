#include "FileReader.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

namespace FileReader
{
    std::vector<std::vector<double>> ReadNumbersFromFileRegex(const std::string &filename)
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
                        if (value == "nan" || value == "NaN")
                        {
                            row.push_back(std::numeric_limits<double>::quiet_NaN());
                        }
                        else if (value == "inf" || value == "Inf" || value == "+inf" || value == "+Inf")
                        {
                            row.push_back(std::numeric_limits<double>::infinity());
                        }
                        else if (value == "-inf" || value == "-Inf")
                        {
                            row.push_back(-std::numeric_limits<double>::infinity());
                        }
                        else
                        {
                            row.push_back(std::stod(value)); // Convert to double
                        }
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

    std::vector<std::vector<double>> ReadNumbersFromFile(const std::string &filename, const std::string &delimiters)
    {
        std::vector<std::vector<double>> data;
        std::ifstream                    file(filename);

        if (!file)
        {
            std::cerr << "Error opening file: " << filename << std::endl;
            return data;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::vector<double> row;
            std::istringstream  lineStream(line);
            std::string         value;

            while (std::getline(lineStream, value, delimiters[0]))
            {
                std::istringstream valueStream(value);
                std::string        token;

                while (std::getline(valueStream, token, ' '))
                {
                    if (!token.empty())
                    {
                        try
                        {
                            if (token == "nan" || token == "NaN")
                            {
                                row.push_back(std::numeric_limits<double>::quiet_NaN());
                            }
                            else if (token == "inf" || token == "Inf" || token == "+inf" || token == "+Inf")
                            {
                                row.push_back(std::numeric_limits<double>::infinity());
                            }
                            else if (token == "-inf" || token == "-Inf")
                            {
                                row.push_back(-std::numeric_limits<double>::infinity());
                            }
                            else
                            {
                                row.push_back(std::stod(token)); // Convert to double
                            }
                        }
                        catch (const std::invalid_argument &)
                        {
                            std::cerr << "Invalid number found in file: " << token << std::endl;
                        }
                        catch (const std::out_of_range &)
                        {
                            std::cerr << "Number out of range in file: " << token << std::endl;
                        }
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
} // namespace FileReader
