#pragma once

#include <string>

class TiXmlElement;

class CXML
{
  public:
    virtual bool XML_ReadWrite(TiXmlElement *&pXML, bool Read /* XML_WRITE XML_READ */, bool bCheckOtherNodes)
    {
        return true;
    };
    virtual std::string CreateText();
    virtual bool        CreateFromText(const char *);
    virtual bool        Load(const std::string FileName = "", bool bReportError = true);
    virtual bool        Save(const std::string FileName = "");

  public: // Debug
    virtual std::string PrintDebugTextString();
    virtual void        PrintDebugConsole(int Level = 0, int MaxLevel = INT_MAX);
};
