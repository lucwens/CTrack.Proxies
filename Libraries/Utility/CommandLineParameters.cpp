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
        return false;
    }
}

// saveToFile implementation
bool CommandLineParameters::saveToFile(const std::string &filename, int indent) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        PrintWarning("Error: Could not open file for writing: {}", filename);
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
        PrintWarning("Error: Could not open file for writing: {}", filename);
        return false;
    }
    try
    {
        ifs >> parameters;
        return true;
    }
    catch (const json::parse_error &e)
    {
        PrintWarning("Error parsing JSON from file {} : {}", filename, e.what());
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

// Private helper function implementation
std::string CommandLineParameters::escapeJsonStringForCommandLine(const std::string &jsonInput)
{
    std::string escaped = jsonInput;
    size_t      pos     = 0;
    while ((pos = escaped.find('\"', pos)) != std::string::npos)
    {
        escaped.replace(pos, 1, "\\\"");
        pos += 2; // Skip the inserted backslash and the quote itself
    }
    return escaped;
}

std::string CommandLineParameters::generateCommandLineArgument(const CommandLineParameters &store)
{
    // First, serialize the store's internal JSON into a string
    std::string jsonString         = store.serialize(-1); // -1 for compact JSON

    // Then, escape this JSON string for the command line
    std::string escapedJsonPayload = escapeJsonStringForCommandLine(jsonString);

    // Finally, enclose the escaped JSON payload in quotes to form a single argument
    return "\"" + escapedJsonPayload + "\"";
}
