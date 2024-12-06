#include "StdAfx.h"
#include "xml.h"
#include "tinyxml.h"
#include "TinyXML_AttributeValues.h"
#include "ErrorHandler.h"

CStringA CXML::CreateText()
{
    CStringA      ReturnString;
    TiXmlElement *pXML = NULL;
    if (XML_ReadWrite(pXML, XML_WRITE, true))
    {
        TiXmlPrinter printer;
        printer.SetIndent("\t");
        pXML->Accept(&printer);
        ReturnString = printer.CStr();
        delete pXML;
    }
    return ReturnString;
}

CStringA CXML::PrintDebugTextString()
{
    return CreateText();
}

bool CXML::Load(LPCSTR FileName, bool bReportError)
{
    TiXmlDocument Doc;
    FILE         *File = nullptr;

    errno_t error      = fopen_s(&File, FileName, "rb");
    if (error != 0)
    {
        if (bReportError)
            HandleErrorGetLastError(ERROR_FILE_OPEN_READ, __FILE__, __LINE__, FileName);
        return false;
    }

    if (!Doc.LoadFile(File, TIXML_ENCODING_UNKNOWN))
    {
        if (bReportError)
            HandleError(ERROR_FILE_OPEN_READ, __FILE__, __LINE__, FileName);
        return false;
    }

    fclose(File);

    TiXmlElement *pXML = Doc.FirstChildElement();

    if (!XML_ReadWrite(pXML, XML_READ, true))
    {
        if (bReportError)
            HandleError(ERROR_FILE_OPEN_READ, __FILE__, __LINE__, FileName);
        return false;
    }
    return true;
}

bool CXML::Save(LPCSTR FileName)
{
    TiXmlDocument Doc(FileName);
    FILE         *File  = nullptr;
    errno_t       error = fopen_s(&File, FileName, "wb");

    if (error != 0)
        return HandleErrorGetLastError(ERROR_FILE_OPEN_WRITE, __FILE__, __LINE__, FileName);

    TiXmlElement *pXML = NULL;
    if (XML_ReadWrite(pXML, XML_WRITE, true))
    {
        Doc.LinkEndChild(pXML);
        if (!Doc.SaveFile(File))
            return HandleErrorGetLastError(ERROR_FILE_OPEN_WRITE, __FILE__, __LINE__, FileName);
        fclose(File);
        return true;
    }
    return false;
}

bool CXML::CreateFromText(const char *XMLText)
{
    TiXmlDocument doc;
    if (!doc.Parse(XMLText))
        return HandleError(ERROR_NODE_FROM_XML, __FILE__, __LINE__, typeid(this).name());
    TiXmlHandle   docHandle(&doc);
    TiXmlElement *pChildElement = docHandle.FirstChildElement().ToElement();
    return XML_ReadWrite(pChildElement, XML_READ, true);
}

void CXML::PrintDebugConsole(int Level, int MaxLevel)
{
#ifdef _DEBUG
    CStringA DebugString = CreateText();
    DebugPrintString(DebugString);
#endif
}
