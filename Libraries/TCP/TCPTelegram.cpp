
#include "TCPTelegram.h"
#include "../XML/TinyXML_AttributeValues.h"
#include "../Utility/Print.h"

std::unique_ptr<TiXmlElement> CreateCommandDetect()
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_DETECT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandEdit()
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_EDIT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

bool DecodeCommand(std::unique_ptr<TiXmlElement> &pXML, std::string &Command, std::map<std::string, std::string> &rParameters);

std::unique_ptr<TiXmlElement> CreateCommandConnect(std::string &iLocalizerName, std::string &iScannerName)
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_CONNECT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND_CONNECT_LOCALIZER, iLocalizerName, false);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND_CONNECT_SCANNER, iScannerName, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandScanStart()
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_SCAN_START);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandScanStop()
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_SCAN_STOP);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandDisconnect()
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_DISCONNECT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandExit()
{
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_EXIT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandTestBigPacket(unsigned long NumLongs)
{
    unsigned long                 PacketSize = NumLongs; // send packet with PacketSize number of unsigned long numbers
    std::string                   Command(TCP_XML_ATTRIB_COMMAND_TEST_BIG);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    GetSetAttribute(ReturnXML.get(), "size", PacketSize, false);
    return ReturnXML;
}

bool DecodeCommand(std::unique_ptr<TiXmlElement> &pXML, std::string &Command, std::map<std::string, std::string> &rParameters)
{
    TiXmlElement *pXMLCommand = FindRecursed(pXML.get(), TCP_XML_TAG_COMMAND);
    if (pXMLCommand)
    {
        TiXmlAttribute *pAttribute = pXMLCommand->FirstAttribute();
        while (pAttribute)
        {
            std::string AttribName  = pAttribute->Name();
            std::string AttribValue = pAttribute->Value();
            rParameters[AttribName] = AttribValue;
            pAttribute              = pAttribute->Next();
        }
        auto iterCommandName = rParameters.find(TCP_XML_ATTRIB_COMMAND);
        if (iterCommandName != rParameters.end())
        {
            Command = iterCommandName->second;
            rParameters.erase(iterCommandName);
            return true;
        }
    }
    return false;
}

unsigned long GetSize(std::vector<std::uint8_t> &Buffer)
{
    unsigned long ReturnSize(0);
    if (Buffer.size() >= 5)
    {
        memcpy(&ReturnSize, Buffer.data() + TCPGRAM_INDEX_SIZE, sizeof(unsigned long));
        return ReturnSize;
    }
    else
        return 0;
}

void SetSize(std::vector<std::uint8_t> &Buffer, unsigned long iSize)
{
    if (Buffer.size() >= 5)
        memcpy(Buffer.data() + TCPGRAM_INDEX_SIZE, &iSize, sizeof(unsigned long));
}

std::uint8_t GetCode(std::vector<std::uint8_t> &Buffer)
{
    std::uint8_t ReturnCode(0);
    if (Buffer.size() >= 5)
        ReturnCode = Buffer[TCPGRAM_INDEX_CODE];
    return ReturnCode;
}

void SetCode(std::vector<std::uint8_t> &Buffer, std::uint8_t iCode)
{
    if (Buffer.size() >= 5)
        Buffer[TCPGRAM_INDEX_CODE] = iCode;
}

//------------------------------------------------------------------------------------------------------------------
/*
CTCPGRam class
*/
//------------------------------------------------------------------------------------------------------------------

CTCPGram::CTCPGram(char *pBytes, unsigned long PackageSize, unsigned char Code)
{
    m_Destination                  = ALL_DESTINATIONS;
    unsigned long TotalPackageSize = TCPGRAM_HEADER_SIZE + PackageSize;
    m_Data.resize(TotalPackageSize);
    SetCode(m_Data, Code);
    SetSize(m_Data, TotalPackageSize);
    memcpy(m_Data.data() + TCPGRAM_INDEX_PAYLOAD, pBytes, PackageSize);
}

void CTCPGram::EncodeText(const std::string &iText, unsigned char Code)
{
    m_Destination                  = ALL_DESTINATIONS;
    unsigned long PackageSize      = static_cast<unsigned long>(sizeof(char) * (iText.size() + 1));
    unsigned long TotalPackageSize = TCPGRAM_HEADER_SIZE + PackageSize;
    m_Data.resize(TotalPackageSize);
    SetCode(m_Data, Code);
    SetSize(m_Data, TotalPackageSize);

    // transfer xml string
    m_Data[TCPGRAM_INDEX_PAYLOAD] = ('\0');
    strcat_s(reinterpret_cast<char *>(m_Data.data() + TCPGRAM_INDEX_PAYLOAD), PackageSize, iText.c_str());
}

CTCPGram::CTCPGram(TiXmlElement &rCommand, unsigned char Code)
{
    std::string XMLText = XMLToString(&rCommand);
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(std::vector<double> &arDoubles)
{
    m_Destination           = ALL_DESTINATIONS;
    size_t NumDoubles       = arDoubles.size();
    size_t PackageSize      = static_cast<unsigned long>(sizeof(double) * NumDoubles);
    size_t TotalPackageSize = TCPGRAM_HEADER_SIZE + PackageSize;
    m_Data.resize(TotalPackageSize);
    SetCode(m_Data, TCPGRAM_CODE_DOUBLES);
    SetSize(m_Data, static_cast<unsigned long>(TotalPackageSize));

    double *pDouble = (double *)(m_Data.data() + TCPGRAM_INDEX_PAYLOAD);
    for (int c = 0; c < NumDoubles; c++)
        pDouble[c] = arDoubles[c];
}

CTCPGram::CTCPGram(std::vector<std::uint8_t> &arBytes, unsigned char Code)
{
    m_Destination           = ALL_DESTINATIONS;
    size_t PackageSize      = arBytes.size();
    size_t TotalPackageSize = TCPGRAM_HEADER_SIZE + PackageSize;
    m_Data.resize(TotalPackageSize);
    SetCode(m_Data, Code);
    SetSize(m_Data, static_cast<unsigned long>(TotalPackageSize));
    memcpy(m_Data.data() + TCPGRAM_INDEX_PAYLOAD, arBytes.data(), PackageSize);
}

CTCPGram::CTCPGram(std::vector<std::uint8_t> &arBytes)
{
    m_Data = std::move(arBytes);
}

CTCPGram::CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code)
{
    std::string XMLText;
    if (rCommand)
        XMLText = XMLToString((rCommand.get()));
    EncodeText(XMLText, Code);
}

#ifdef _MANAGED

CTCPGram::CTCPGram(cliext::vector<double> arDoubles)
{
    m_Destination           = ALL_DESTINATIONS;
    size_t NumDoubles       = arDoubles.size();
    size_t PackageSize      = sizeof(double) * NumDoubles;
    size_t TotalPackageSize = TCPGRAM_HEADER_SIZE + PackageSize;
    m_Data.resize(TotalPackageSize);
    SetCode(m_Data, TCPGRAM_CODE_DOUBLES);
    SetSize(m_Data, static_cast<unsigned long>(TotalPackageSize));
    double *pDouble = (double *)(m_Data.data() + TCPGRAM_INDEX_PAYLOAD);
    for (int c = 0; c < NumDoubles; c++)
        pDouble[c] = arDoubles[c];
}

#endif

void CTCPGram::CopyFrom(std::unique_ptr<CTCPGram> &rFrom)
{
    m_Destination = rFrom->m_Destination;
    m_Data        = rFrom->m_Data;
}

unsigned char CTCPGram::GetCode()
{
    if (m_Data.size() >= 5)
        return ::GetCode(m_Data);
    else
        return TCPGRAM_CODE_INVALID;
}

unsigned long CTCPGram::GetSize()
{
    if (m_Data.size() >= 5)
        return ::GetSize(m_Data);
    else
        return 0;
}

const char *CTCPGram::GetText()
{
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS)
        return NULL;
    return reinterpret_cast<const char *>(m_Data.data() + TCPGRAM_INDEX_PAYLOAD);
}

std::unique_ptr<TiXmlElement> CTCPGram::GetXML()
{
    std::unique_ptr<TiXmlElement> ReturnXML;

    // get the oldest telegram and remove from the receive buffer
    const char *pXMLString = GetText();
    if (pXMLString != nullptr)
    {
        TiXmlDocument doc;
        if (strlen(pXMLString) != 0)
        {
            doc.Parse(pXMLString);
            if (doc.Error())
            {
                std::string ErrorDescription = fmt::format("TCP Error ({},{}): the XML contents could not be decoded :{} \n{}", doc.ErrorRow(), doc.ErrorCol(),
                                                           doc.ErrorDesc(), pXMLString);
                throw std::exception(ErrorDescription.c_str());
            }
        }
        TiXmlElement *pXML = doc.FirstChildElement();
        RemoveElement(pXML); // remove from doc, otherwise doc's destructor will kill our XML
        ReturnXML.reset(pXML);
    }
    return ReturnXML;
}

void CTCPGram::Clear()
{
    m_Data.clear();
}
