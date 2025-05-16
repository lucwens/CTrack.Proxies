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

const std::string MessageTypeCommand       = "Command";
const std::string MessageTypeCommandReturn = "CommandReturn";
const std::string MessageTypeEvent         = "Event";
const std::string MessageTypeWarning       = "Warning";

inline void to_json(json &j, const MessageType &messageType)
{
    switch (messageType)
    {
        case MessageType::Command:
            j = MessageTypeCommand;
            break;
        case MessageType::CommandReturn:
            j = MessageTypeCommandReturn;
            break;
        case MessageType::Event:
            j = MessageTypeEvent;
            break;
        case MessageType::Warning:
            j = MessageTypeWarning;
            break;
        default:
            j = "Unknown";
            break; // Or throw an error
    }
}

inline void from_json(const json &j, MessageType &messageType)
{
    if (j.is_string())
    {
        const std::string s = j.get<std::string>();
        if (s == MessageTypeCommand)
            messageType = MessageType::Command;
        else if (s == MessageTypeCommandReturn)
            messageType = MessageType::CommandReturn;
        else if (s == MessageTypeEvent)
            messageType = MessageType::Event;
        else if (s == MessageTypeWarning)
            messageType = MessageType::Warning;
        else
            throw json::type_error::create(317, "invalid string for messageType: " + s, &j); // Error for unknown string
    }
    else
    {
        throw json::type_error::create(317, "messageType must be a string in JSON", &j); // Error if not a string
    }
}

class CMessage
{

  public:
    CMessage() = default;
    CMessage(const std::string &ID, const MessageType &messageType);
    virtual ~CMessage();

  public: // serialization
    virtual std::string Serialize() const;
    virtual void        Deserialize(const std::string &data);

  public: // accessors
    std::string GetID() const;
    void        SetID(const std::string &id);
    MessageType GetType() const;
    void        SetType(MessageType type);
    json       &GetParameters();

  protected:
    json m_Parameters; // parameters can be accessed as a JSON object
};

const std::string KeyWordID   = "ID";   // ID of the message
const std::string KeyWordType = "Type"; // Type of the message
