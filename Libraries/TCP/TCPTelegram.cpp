
#include "TCPTelegram.h"
#include "../XML/TinyXML_AttributeValues.h"

unsigned long CalcBufferSize(unsigned long NScanPoints) noexcept
{
    unsigned long ReturnSize = 2 /*buttonid + button state*/;
    ReturnSize += sizeof(CycleItem6D) /*position of localizer*/;
    ReturnSize += sizeof(unsigned long) /*number of scans*/;
    ReturnSize += NScanPoints * sizeof(ScanPoint3D) /*variable length array of scanpoints*/;

    return ReturnSize;
}

void EncodeScanToBuffer(char *pBuffer, size_t BufferSize, char &ButtonID, char &ButtonState, CycleItem6D &Pose, std::deque<ScanPoint3D> &ScanPoints) noexcept
{
    char *pBufferPos{pBuffer};
    /* the buffer is composed of
    N scanpoints [4 bytes]
    Pose of scanner [sizeof(CycleItem6D) bytes]
    N x ScanPoint3D [N x sizeof(ScanPoint3D)]
    */
    // get button id and state
    memcpy(pBufferPos, &ButtonID, sizeof(char));
    pBufferPos += sizeof(char);
    memcpy(pBufferPos, &ButtonState, sizeof(char));
    pBufferPos += sizeof(char);

    // get pose
    memcpy(pBufferPos, &Pose, sizeof(CycleItem6D));
    pBufferPos += sizeof(CycleItem6D);

    // get num of scan points
    unsigned long N = ScanPoints.size();
    memcpy(pBufferPos, &N, sizeof(unsigned long));
    pBufferPos += sizeof(unsigned long);

    // get scan points
    for (auto &ScanPoint : ScanPoints)
    {
        // get pose
        memcpy(pBufferPos, &ScanPoint, sizeof(ScanPoint3D));
        pBufferPos += sizeof(ScanPoint3D);
        ScanPoints.push_back(ScanPoint);
    }
}

unsigned long DecodeScanFromBuffer(/*input*/ char *pBuffer, size_t BufferSize,
                                   /*output*/ char &ButtonID, char &ButtonState, CycleItem6D &Pose, std::deque<ScanPoint3D> &ScanPoints) noexcept
{
    char *pBufferPos{pBuffer};
    /* the buffer is composed of
    N scanpoints [4 bytes]
    Pose of scanner [sizeof(CycleItem6D) bytes]
    N x ScanPoint3D [N x sizeof(ScanPoint3D)]
    */
    // get button id and state
    memcpy(&ButtonID, pBufferPos, sizeof(char));
    pBufferPos += sizeof(char);
    memcpy(&ButtonState, pBufferPos, sizeof(char));
    pBufferPos += sizeof(char);

    // get pose as 4x4 matrix
    memcpy(&Pose, pBufferPos, sizeof(CycleItem6D));
    pBufferPos += sizeof(CycleItem6D);

    // get num of scan points
    unsigned long N{0};
    memcpy(&N, pBufferPos, sizeof(unsigned long));
    pBufferPos += sizeof(unsigned long);

    // get scan points
    ScanPoints.clear();
    ScanPoint3D ScanPoint;
    for (unsigned long I = 0; I < N; I++)
    {
        // get pose
        memcpy(&ScanPoint, pBufferPos, sizeof(ScanPoint3D));
        pBufferPos += sizeof(ScanPoint3D);
        ScanPoints.push_back(ScanPoint);
    }
    return N;
}

std::unique_ptr<TiXmlElement> CreateCommandDetect()
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_DETECT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandEdit()
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_EDIT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

bool DecodeCommand(std::unique_ptr<TiXmlElement> &pXML, std::string &Command, std::map<std::string, std::string> &rParameters);

std::unique_ptr<TiXmlElement> CreateCommandConnect(std::string &iLocalizerName, std::string &iScannerName)
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_CONNECT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND_CONNECT_LOCALIZER, iLocalizerName, false);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND_CONNECT_SCANNER, iScannerName, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandScanStart()
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_SCAN_START);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandScanStop()
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_SCAN_STOP);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandDisconnect()
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_DISCONNECT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandExit()
{
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_EXIT);
    std::unique_ptr<TiXmlElement> ReturnXML = std::make_unique<TiXmlElement>(TCP_XML_TAG_COMMAND);
    GetSetAttribute(ReturnXML.get(), TCP_XML_ATTRIB_COMMAND, Command, false);
    return ReturnXML;
}

std::unique_ptr<TiXmlElement> CreateCommandTestBigPacket(unsigned long NumLongs)
{
    unsigned long                 PacketSize = NumLongs; // send packet with PacketSize number of unsigned long numbers
    std::string                      Command(TCP_XML_ATTRIB_COMMAND_TEST_BIG);
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
            std::string AttribName     = pAttribute->Name();
            std::string AttribValue    = pAttribute->Value();
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

unsigned long GetSize(char *pBuffer)
{
    unsigned long ReturnSize(0);
    if (pBuffer)
    {
        memcpy(&ReturnSize, &pBuffer[TCPGRAM_INDEX_SIZE], sizeof(unsigned long));
        return ReturnSize;
    }
    else
        return 0;
}

void SetSize(char *pBuffer, unsigned long iSize)
{
    memcpy(&pBuffer[TCPGRAM_INDEX_SIZE], &iSize, sizeof(unsigned long));
}

int GetCode(char *pBuffer)
{
    unsigned char ReturnCode(0);
    memcpy(&ReturnCode, &pBuffer[TCPGRAM_INDEX_CODE], sizeof(unsigned char));
    return ReturnCode;
}

void SetCode(char *pBuffer, int iCode)
{
    memcpy(&pBuffer[TCPGRAM_INDEX_CODE], &iCode, sizeof(unsigned char));
}

//------------------------------------------------------------------------------------------------------------------
/*
CTCPGRam class
*/
//------------------------------------------------------------------------------------------------------------------

CTCPGram::CTCPGram(char *pFromReadBuffer)
{
    m_Destination = ALL_DESTINATIONS;
    m_PackageSize = ::GetSize(pFromReadBuffer);
    m_pData.reset(pFromReadBuffer);
}

CTCPGram::CTCPGram(char *pBytes, unsigned long NumBytes, unsigned char Code)
{
    m_Destination = ALL_DESTINATIONS;
    m_PackageSize = TCPGRAM_HEADER_SIZE + NumBytes;
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), Code);
    SetSize(m_pData.get(), m_PackageSize);
    memcpy(&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD]), pBytes, NumBytes);
}

void CTCPGram::EncodeText(const std::string& iText, unsigned char Code)
{
    m_Destination = ALL_DESTINATIONS;
    m_PackageSize = TCPGRAM_HEADER_SIZE + sizeof(char) * (iText.size() + 1);
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), Code);
    SetSize(m_pData.get(), m_PackageSize);

    // transfer xml string
    m_pData.get()[TCPGRAM_INDEX_PAYLOAD] = ('\0');
    strcat_s(&m_pData.get()[TCPGRAM_INDEX_PAYLOAD],m_PackageSize ,iText.c_str());
}

CTCPGram::CTCPGram(TiXmlElement &rCommand, unsigned char Code)
{
    std::string XMLText = XML_To_String(&rCommand);
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code)
{
    std::string XMLText;
    if (rCommand)
        XMLText = XML_To_String((rCommand.get()));
    EncodeText(XMLText, Code);
}

void CTCPGram::CopyFrom(std::unique_ptr<CTCPGram> &rFrom)
{
    m_PackageSize = rFrom->m_PackageSize;
    m_Destination = rFrom->m_Destination;
    m_pData.reset(new char[m_PackageSize]);
    memcpy(m_pData.get(), rFrom->m_pData.get(), m_PackageSize);
}

unsigned char CTCPGram::GetCode()
{
    if (m_pData != nullptr && m_PackageSize > 0)
        return ::GetCode(m_pData.get());
    else
        return TCPGRAM_CODE_INVALID;
}

unsigned long CTCPGram::GetSize()
{
    if (m_pData != nullptr && m_PackageSize > 0)
        return ::GetSize(m_pData.get());
    else
        return 0;
}

const char *CTCPGram::GetText()
{
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS)
        return NULL;
    return (const char *)(&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD]));
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
            if (!doc.Parse(pXMLString))
                throw std::exception("TCP Error : the XML contents could not be decoded");
        TiXmlElement *pXML = doc.FirstChildElement();
        RemoveElement(pXML); // remove from doc, otherwise doc's destructor will kill our XML
        ReturnXML.reset(pXML);
    }
    return ReturnXML;
}

void CTCPGram::Clear()
{
    m_pData.reset();
    m_PackageSize = 0;
}
