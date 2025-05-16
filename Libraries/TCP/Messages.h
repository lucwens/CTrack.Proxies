#pragma once
#include "Node_Base.h"
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

class CMessage : public CNode
{
    CMessage();

  public:
    virtual ~CMessage();

  public: // serialization
    bool XML_ReadWrite(TiXmlElement *&pXML, bool Read /* XML_WRITE XML_READ */, bool bCheckOtherNodes) override;
    void CopyFrom(CNode *) override;

  public: // accessors
    std::unique_ptr<TiXmlElement> GetPayloadXML();
    void                          SetPayloadXML(std::unique_ptr<TiXmlElement> &val);
    CString                       GetPayload();
    void                          SetPayload(CString val);

  protected:
    std::string m_ID;         // text containing the carried information
    MessageType m_Type;       // type of message (Command, CommandReturn, Event, Warning)
    json        m_Parameters; // parameters can be accessed as a JSON object
};
