#pragma once

#include <string>

class TiXmlElement;

class CXML
{
  public:
    virtual bool XML_ReadWrite(TiXmlElement *&pXML, bool Read )
    {
        return true;
    };
    virtual std::string CreateText();
    virtual bool        CreateFromText(const char *);
    virtual bool        Load(const std::string& FileName = "");
    virtual bool        Save(const std::string& FileName = "");

  public: // Debug
    virtual std::string PrintDebugTextString();
};
