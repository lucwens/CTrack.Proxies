#include "CommandLineParameters.h" // Include the header file for the class declaration
#include <iostream>
#include <fstream>
#include <stdexcept> // For std::runtime_error or other exceptions if needed

// Default constructor implementation
CommandLineParameters::CommandLineParameters() = default;

// Constructor from command-line arguments implementation
CommandLineParameters::CommandLineParameters(int argc, char *argv[])
{
    if (argc > 1)
    {
        std::string inputString = argv[1];
        if (deserialize(inputString))
        {
            initializedSuccessfullyFromJson_ = true;
        }
        else
        {
            std::cerr << "CommandLineParameters: Command-line input is not a valid JSON string." << std::endl;
        }
    }
    else
    {
        std::cout << "CommandLineParameters: No command-line argument provided. Initializing empty store." << std::endl;
    }
}

// isInitializedFromJson implementation
bool CommandLineParameters::isInitializedFromJson() const
{
    return initializedSuccessfullyFromJson_;
}

// set overloads implementation
void CommandLineParameters::set(const std::string &key, int value)
{
    parameters[key] = value;
}

void CommandLineParameters::set(const std::string &key, uint64_t value)
{
    parameters[key] = value;
}

void CommandLineParameters::set(const std::string &key, double value)
{
    parameters[key] = value;
}

void CommandLineParameters::set(const std::string &key, bool value)
{
    parameters[key] = value;
}

void CommandLineParameters::set(const std::string &key, const std::string &value)
{
    parameters[key] = value;
}

// get overloads implementation
int CommandLineParameters::getInt(const std::string &key, int defaultValue) const
{
    if (parameters.count(key) && parameters[key].is_number_integer())
    {
        return parameters[key].get<int>();
    }
    return defaultValue;
}

uint64_t CommandLineParameters::getUint64(const std::string &key, uint64_t defaultValue) const
{
    // nlohmann::json uses `is_number_unsigned()` for unsigned integers
    if (parameters.count(key) && parameters[key].is_number_unsigned())
    {
        return parameters[key].get<uint64_t>();
    }
    return defaultValue;
}

double CommandLineParameters::getDouble(const std::string &key, double defaultValue) const
{
    if (parameters.count(key) && parameters[key].is_number())
    {
        return parameters[key].get<double>();
    }
    return defaultValue;
}

bool CommandLineParameters::getBool(const std::string &key, bool defaultValue) const
{
    if (parameters.count(key) && parameters[key].is_boolean())
    {
        return parameters[key].get<bool>();
    }
    return defaultValue;
}

std::string CommandLineParameters::getString(const std::string &key, const std::string &defaultValue) const
{
    if (parameters.count(key) && parameters[key].is_string())
    {
        return parameters[key].get<std::string>();
    }
    return defaultValue;
}

// contains implementation
bool CommandLineParameters::contains(const std::string &key) const
{
    return parameters.count(key) > 0;
}

// remove implementation
void CommandLineParameters::remove(const std::string &key)
{
    parameters.erase(key);
}

// clear implementation
void CommandLineParameters::clear()
{
    parameters.clear();
}

// serialize implementation
std::string CommandLineParameters::serialize(int indent) const
{
    return parameters.dump(indent);
}

// deserialize implementation
bool CommandLineParameters::deserialize(const std::string &jsonString)
{
    try
    {
        parameters = json::parse(jsonString);
        return true;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "Error deserializing JSON string: " << e.what() << std::endl;
        return false;
    }
}

// saveToFile implementation
bool CommandLineParameters::saveToFile(const std::string &filename, int indent) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    ofs << parameters.dump(indent);
    ofs.close();
    return true;
}

// loadFromFile implementation
bool CommandLineParameters::loadFromFile(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
    {
        std::cerr << "Error: Could not open file for reading: " << filename << std::endl;
        return false;
    }
    try
    {
        ifs >> parameters;
        return true;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "Error parsing JSON from file '" << filename << "': " << e.what() << std::endl;
        return false;
    }
}

// getJsonObject implementations
json &CommandLineParameters::getJsonObject()
{
    return parameters;
}

const json &CommandLineParameters::getJsonObject() const
{
    return parameters;
}
