#include "Messages.h"
#include "../Utility/Print.h"


CMessage::CMessage(const std::string &ID, const MessageType &messageType)
{
    m_Parameters[KeyWordID]   = ID;
    m_Parameters[KeyWordType] = messageType;
}

CMessage::~CMessage()
{
}

std::string CMessage::Serialize() const
{
    std::string serializedMessage = m_Parameters.dump();
    return serializedMessage;
}

void CMessage::Deserialize(const std::string &data)
{
    try
    {
        m_Parameters = json::parse(data);
    }
    catch (const std::exception &e)
    {
        // Handle parsing error
        PrintError("Error parsing message:{}", e.what());
    };
}

 std::string CMessage::GetID() const
{
    if (m_Parameters.contains(KeyWordID))
    {
        return m_Parameters[KeyWordID].get<std::string>();
    }
    else
    {
        throw std::runtime_error("ID not found in message parameters.");
    }
}

 void CMessage::SetID(const std::string &id) 
{
    if (m_Parameters.contains(KeyWordID))
    {
        m_Parameters[KeyWordID] = id;
    }
    else
    {
        throw std::runtime_error("ID not found in message parameters.");
    }
}

MessageType CMessage::GetType() const
{
    if (m_Parameters.contains(KeyWordType))
    {
        return m_Parameters[KeyWordType].get<MessageType>();
    }
    else
    {
        throw std::runtime_error("Type not found in message parameters.");
    }
}

void CMessage::SetType(MessageType type)
{
    if (m_Parameters.contains(KeyWordType))
    {
        m_Parameters[KeyWordType] = type;
    }
    else
    {
        throw std::runtime_error("Type not found in message parameters.");
    }
}

json &CMessage::GetParameters()
{
    return m_Parameters;
}
