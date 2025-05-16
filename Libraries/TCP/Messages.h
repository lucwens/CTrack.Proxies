#pragma once
#include <nlohmann/json.hpp>

// Alias for convenience
using json = nlohmann::json; // Common alias

enum class MessageType
{
    Command,
    CommandReturn,
    Event,
    Warning
};

class CMessage 
{

  public:
    CMessage();
    virtual ~CMessage();

  public: // serialization

  public: // accessors

  protected:
    std::string m_ID;         // text containing the carried information
    MessageType m_Type;       // type of message (Command, CommandReturn, Event, Warning)
    json        m_Parameters; // parameters can be accessed as a JSON object
};
