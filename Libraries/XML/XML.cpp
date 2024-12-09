#include "xml.h"
#include "tinyxml.h"
#include "TinyXML_AttributeValues.h"

std::string CXML::CreateText()
{
    std::string   ReturnString;
    TiXmlElement *pXML = NULL;
    if (XML_ReadWrite(pXML, XML_WRITE))
    {
        TiXmlPrinter printer;
        printer.SetIndent("\t");
        pXML->Accept(&printer);
        ReturnString = printer.CStr();
        delete pXML;
    }
    return ReturnString;
}

std::string CXML::PrintDebugTextString()
{
    return CreateText();
}

bool CXML::Load(const std::string &FileName)
{
    TiXmlDocument Doc;
    FILE         *File = nullptr;

    errno_t error      = fopen_s(&File, FileName.c_str(), "rb");
    if (error != 0)
    {
        return false;
    }

    if (!Doc.LoadFile(File, TIXML_ENCODING_UNKNOWN))
    {
        return false;
    }

    fclose(File);

    TiXmlElement *pXML = Doc.FirstChildElement();

    if (!XML_ReadWrite(pXML, XML_READ))
    {
        return false;
    }
    return true;
}

bool CXML::Save(const std::string &FileName)
{
    TiXmlDocument Doc(FileName.c_str());
    FILE         *File  = nullptr;
    errno_t       error = fopen_s(&File, FileName.c_str(), "wb");

    if (error != 0)
        return false;

    TiXmlElement *pXML = NULL;
    if (XML_ReadWrite(pXML, XML_WRITE))
    {
        Doc.LinkEndChild(pXML);
        if (!Doc.SaveFile(File))
            return false;
        fclose(File);
        return true;
    }
    return false;
}

bool CXML::CreateFromText(const char *XMLText)
{
    TiXmlDocument doc;
    if (!doc.Parse(XMLText))
        return false;
    TiXmlHandle   docHandle(&doc);
    TiXmlElement *pChildElement = docHandle.FirstChildElement().ToElement();
    return XML_ReadWrite(pChildElement, XML_READ);
}
