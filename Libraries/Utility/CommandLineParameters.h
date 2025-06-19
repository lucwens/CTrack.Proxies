
#include "Print.h"
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp> // Assuming nlohmann/json.hpp is available

// Alias for convenience
using json = nlohmann::json;

class CommandLineParameters
{
  private:
    json parameters;
    bool      initializedSuccessfullyFromJson_ = false;
    static std::string escapeJsonStringForCommandLine(const std::string &jsonInput);

  public:
    CommandLineParameters();
    CommandLineParameters(int argc, char *argv[]);

    bool isInitializedFromJson() const;

    void set(const std::string &key, int value);
    void set(const std::string &key, uint64_t value);
    void set(const std::string &key, double value);
    void set(const std::string &key, bool value);
    void set(const std::string &key, const std::string &value);

    template <typename T> void setGeneric(const std::string &key, const T &value);

    int         getInt(const std::string &key, int defaultValue) const;
    uint64_t    getUint64(const std::string &key, uint64_t defaultValue) const;
    double      getDouble(const std::string &key, double defaultValue) const;
    bool        getBool(const std::string &key, bool defaultValue) const;
    std::string getString(const std::string &key, const std::string &defaultValue) const;

    template <typename T> T getGeneric(const std::string &key, const T &defaultValue) const;

    bool contains(const std::string &key) const;
    void remove(const std::string &key);
    void clear();

    std::string serialize(int indent = 2) const;
    bool        deserialize(const std::string &jsonString);
    bool        saveToFile(const std::string &filename, int indent = 2) const;
    bool        loadFromFile(const std::string &filename);

    json       &getJsonObject();
    const json &getJsonObject() const;

    static std::string generateCommandLineArgument(const CommandLineParameters &store);
};

// Template method definitions must be in the header or in a separate .tpp file
// if you want to keep them separated but still visible to the compiler
// at the point of instantiation.
template <typename T> void CommandLineParameters::setGeneric(const std::string &key, const T &value)
{
    parameters[key] = value;
}

template <typename T> T CommandLineParameters::getGeneric(const std::string &key, const T &defaultValue) const
{
    if (parameters.count(key))
    {
        try
        {
            return parameters[key].get<T>();
        }
        catch (const json::type_error &e)
        {
            PrintWarning("Warning: Type mismatch for key {} : {}", key, e.what());
        }
    }
    return defaultValue;
}
