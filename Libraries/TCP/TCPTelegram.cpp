
#ifdef CTRACK
#include "stdafx.h"
#include "DeviceOutputTCP.h"
#include "../version.h"
#else
#include "../../../version.h"
#endif

#include "TCPTelegram.h"
#include "../XML/TinyXML_AttributeValues.h"
#include "../Utility/Print.h"
#include "../Utility/StringUtilities.h"
#include "../XML/proxyKeywords.h"
#include <fmt/format.h>

//------------------------------------------------------------------------------------------------------------------
/*
CTCPGRam class
*/
//------------------------------------------------------------------------------------------------------------------

CTCPGram::CTCPGram(TMessageHeader &messageHeader, std::vector<char> &dataBuffer)
{
    m_MessageHeader = messageHeader;
    m_Data          = std::move(dataBuffer);
    m_MessageHeader.SetPayloadSize(m_Data.size());
    messageHeader.Reset();
}

CTCPGram::CTCPGram(std::vector<char> &dataBuffer)
{
    m_Data = std::move(dataBuffer);
}

CTCPGram::CTCPGram(char *pBytes, size_t PackageSize, unsigned char Code)
{
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), pBytes, PackageSize);
}

void CTCPGram::EncodeText(const std::string &iText, unsigned char Code)
{
    size_t PackageSize = static_cast<unsigned long>(sizeof(char) * (iText.size() + 1));
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);

    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), iText.c_str(), PackageSize);
}

void CTCPGram::EncodeDoubleArray(std::vector<double> &iDoubleArray)
{
    std::uint16_t NumChannels = iDoubleArray.size();
    size_t        PackageSize = sizeof(double) * NumChannels + sizeof(std::uint16_t);
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(TCPGRAM_CODE_DOUBLES);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), &NumChannels, sizeof(std::uint16_t));
    memcpy(m_Data.data() + sizeof(std::uint16_t), iDoubleArray.data(), sizeof(double) * NumChannels);
}

#ifdef _MANAGED

CTCPGram::CTCPGram(cliext::vector<double> arDoubles)
{
    size_t NumChannels = arDoubles.size();
    size_t PackageSize = sizeof(double) * NumChannels + sizeof(std::uint16_t);
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(TCPGRAM_CODE_DOUBLES);
    m_Data.resize(PackageSize);

    memcpy(m_Data.data(), &NumChannels, sizeof(std::uint16_t));

    double *pDouble = reinterpret_cast<double *>(m_Data.data() + sizeof(std::uint16_t));
    for (size_t c = 0; c < NumChannels; c++)
        pDouble[c] = arDoubles[c];
}

#endif

bool CTCPGram::GetDoubleArray(std::vector<double> &arDoubles)
{
    unsigned char Code = GetCode();

    if (Code != TCPGRAM_CODE_DOUBLES)
        return false;
    arDoubles.clear();

    std::uint16_t NumChannels = 0;
    memcpy(&NumChannels, m_Data.data(), sizeof(std::uint16_t));
    assert(NumChannels == (m_Data.size() - sizeof(std::uint16_t)) / sizeof(double));

    double *pDouble = reinterpret_cast<double *>(m_Data.data() + sizeof(std::uint16_t));
    arDoubles.resize(NumChannels);
    for (size_t c = 0; c < NumChannels; c++)
        arDoubles[c] = pDouble[c];
    return true;
}

bool CTCPGram::GetDoubleQue(std::deque<double> &queDoubles)
{
    std::vector<double> arDoubles;
    if (!GetDoubleArray(arDoubles))
        return false;

    queDoubles.clear();
    for (auto &dValue : arDoubles)
        queDoubles.emplace_back(dValue);
    return true;
}

CTCPGram::CTCPGram(TiXmlElement &rCommand, unsigned char Code)
{
    std::string XMLText = XMLToString(&rCommand);
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(std::vector<double> &arDoubles)
{
    EncodeDoubleArray(arDoubles);
}

CTCPGram::CTCPGram(const std::vector<char> &arBytes, unsigned char Code)
{
    size_t PackageSize = arBytes.size();
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), arBytes.data(), PackageSize);
}

CTCPGram::CTCPGram(const std::string &string, unsigned char Code)
{
    EncodeText(string, Code);
}

CTCPGram::CTCPGram(std::vector<char> &&arBytes, unsigned char Code)
{
    size_t PackageSize = arBytes.size();
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
    m_Data = std::move(arBytes);
}

CTCPGram::CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code)
{
    std::string XMLText;
    if (rCommand)
        XMLText = XMLToString((rCommand.get()));
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(const std::exception &e)
{
    std::unique_ptr<TiXmlElement> xml     = std::make_unique<TiXmlElement>(TAG_ERROR);
    std::string                   XMLText = e.what();
    EncodeText(XMLText, TCPGRAM_CODE_ERROR);
}

CTCPGram::CTCPGram(const CTrack::Message & message)
{
    EncodeText(message.Serialize(), TCPGRAM_CODE_MESSAGE);
}

void CTCPGram::CopyFrom(std::unique_ptr<CTCPGram> &rFrom)
{
    m_Destination   = rFrom->m_Destination;
    m_MessageHeader = rFrom->m_MessageHeader;
    m_Data          = rFrom->m_Data;
}

unsigned char CTCPGram::GetCode()
{
    return m_MessageHeader.GetCode();
}

void CTCPGram::SetCode(unsigned char iCode)
{
    m_MessageHeader.SetCode(iCode);
}

std::uint32_t CTCPGram::GetSize()
{
    return m_MessageHeader.GetPayloadSize();
}

std::string CTCPGram::GetText()
{
    std::string   ReturnString;
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS && DataType != TCPGRAM_CODE_MESSAGE)
        return "";
    ReturnString.assign(m_Data.begin(), m_Data.end());
    return ReturnString;
}

bool CTCPGram::GetString(std::string &rString)
{
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS && DataType != TCPGRAM_CODE_MESSAGE)
        return false;
    rString = GetText();
    return true;
}

std::vector<char> CTCPGram::GetData()
{
    return std::move(m_Data);
}

std::unique_ptr<TiXmlElement> CTCPGram::GetXML()
{
    std::unique_ptr<TiXmlElement> ReturnXML;

    // get the oldest telegram and remove from the receive buffer
    std::string   XMLString = GetText();
    TiXmlDocument doc;
    if (XMLString.size() != 0)
    {
        doc.Parse(XMLString.c_str());
        if (doc.Error())
        {
            std::string ErrorDescription =
                fmt::format("TCP Error ({},{}): the XML contents could not be decoded :{} \n{}", doc.ErrorRow(), doc.ErrorCol(), doc.ErrorDesc(), XMLString);
            throw std::exception(ErrorDescription.c_str());
        }
    }
    TiXmlElement *pXML = doc.FirstChildElement();
    RemoveElement(pXML); // remove from doc, otherwise doc's destructor will kill our XML
    ReturnXML.reset(pXML);
    return ReturnXML;
}


bool CTCPGram::GetMessage(CTrack::Message &message)
{
    if (GetCode() != TCPGRAM_CODE_MESSAGE)
        return false;
    message.Deserialize(GetText());
    return true;
}

#ifdef CTRACK

CTCPGram::CTCPGram(CXML *ipXML, unsigned char Code)
{
    std::string XMLText;
    m_Destination = ALL_DESTINATIONS;
    m_Source      = 0;
    if (ipXML != nullptr)
        XMLText = ipXML->CreateText();
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(ChartIndex FrameNumber, HMatrix &rhInput)
{
    std::vector<double> arDoubles;
    WORD                NumChannels = rhInput.Cols();
    arDoubles.resize(NumChannels);
    for (int c = 0; c < NumChannels; c++)
    {
        arDoubles[c] = rhInput(FrameNumber, c);
    }
    m_Destination = ALL_DESTINATIONS;
    EncodeDoubleArray(arDoubles);
}

bool CTCPGram::GetMm(Mm &rMatrix)
{
    std::vector<double> arDoubles;
    if (GetDoubleArray(arDoubles))
    {
        size_t NumDoubles = arDoubles.size();
        for (int c = 0; c < NumDoubles; c++)
            rMatrix.r(1, c + 1) = arDoubles[c];

        if (NumDoubles != rMatrix.cols() && rMatrix.rows() != 1)
            rMatrix = zeros(1, NumDoubles);

        return true;
    }
    return false;
}

#define TAG_CONFIGURATION  _T("CONFIGURATION")
#define TAG_CHANNEL        _T("CHANNEL")
#define ATTRIB_MEAS_FREQ   _T("meas_freq")
#define ATTRIB_CONFIG_FILE _T("config_file")
#define ATTRIB_NAME        _T("name")
#define ATTRIB_UNIT        _T("unit")
#define ATTRIB_ENCODING    _T("encoding")
#define ATTRIB_VERSION     _T("version")

CTCPGram::CTCPGram(HMatrix &rhInput, SOCKET iDestination)
{
    //
    // create an XML for the channels and units
    std::unique_ptr<TiXmlElement> pXML           = std::make_unique<TiXmlElement>(TAG_CONFIGURATION);
    double                        MeasFreq       = StateManager.GetMeasurementFrequency();
    CConfiguration               *pConfiguration = StateManager.GetConfiguration();
    std::string                   ConfigName     = pConfiguration->GetName();
    std::string                   VersionString  = fmt::format("{}_{}", GIT_TAG, GIT_HASH);
    GetSetAttribute(pXML.get(), ATTRIB_MEAS_FREQ, MeasFreq, /*Read*/ false);
    GetSetAttribute(pXML.get(), ATTRIB_CONFIG_FILE, ConfigName, /*Read*/ false);
    GetSetAttribute(pXML.get(), ATTRIB_VERSION, VersionString, /*Read*/ false);

    for (int c = 0; c < rhInput.Cols(); c++)
    {
        CConfigChannelOutput *pOutputChannel = dynamic_cast<CConfigChannelOutput *>(rhInput.GetChannel(c));
        assert(pOutputChannel);
        if (pOutputChannel)
        {
            CConfigChannel *pInputChannel = pOutputChannel->GetInputChannel();
            if (pInputChannel)
            {
                CString Name;
                CString Unit     = pInputChannel->GetUnit(false); // TODO : UNIT
                int     Encoding = pInputChannel->GetEncoding();
                pInputChannel->GetPath(Name);
                TiXmlElement *pXMLChannel = new TiXmlElement(TAG_CHANNEL);
                GetSetAttribute(pXMLChannel, ATTRIB_NAME, Name, /*Read*/ false);
                GetSetAttribute(pXMLChannel, ATTRIB_UNIT, Unit, /*Read*/ false);
                GetSetAttribute(pXMLChannel, ATTRIB_ENCODING, Encoding, /*Read*/ false);
                pXML->LinkEndChild(pXMLChannel);
            }
        }
    }

    std::string XMLText = XML_To_String(pXML.get());
    EncodeText(XMLText, TCPGRAM_CODE_CONFIGURATION);
}

CTCPGram::CTCPGram(std::string &CommandReturn)
{
    EncodeText(std::string(CommandReturn), TCPGRAM_CODE_COMMAND);
}

const int STATUS_CODE_IDLE    = 0;
const int STATUS_CODE_RUNNING = 1;
const int STATUS_CODE_LOGGING = 2;
const int STATUS_CODE_ERROR   = 3;

CTCPGram::CTCPGram(CState *pState)
{
    std::string TextString;
    char        StatusCode = STATUS_CODE_IDLE;
    TextString.clear();
    if (pState->TypeEquals(typeid(CStateError)))
    {
        TextString = std::string(StateManager.GetErrorMessage());
        StatusCode = STATUS_CODE_ERROR;
    }
    else if (pState->TypeEquals(typeid(CStateBuffering)))
    {
        CStateBuffering *pStateBuffering = dynamic_cast<CStateBuffering *>(pState);
        if (pStateBuffering)
        {
            TextString = fmt::format("{}\\{};{};{}", std::string(pStateBuffering->m_BufProjectName), std::string(pStateBuffering->m_BufTestName),
                                     pStateBuffering->m_BufRunTime, pStateBuffering->m_BufTotal);
        }
        StatusCode = STATUS_CODE_LOGGING;
    }
    else if (pState->TypeEquals(typeid(CStateRunning)))
        StatusCode = STATUS_CODE_RUNNING;

    TextString.insert(TextString.begin(), StatusCode);

    SetCode(TCPGRAM_CODE_STATUS);
    SetPayloadSize(TextString.size());
    m_Data.swap(*reinterpret_cast<std::vector<char> *>(&TextString));
}

CTCPGram::CTCPGram(const std::string &iProjectName, const std::string &iTestName)
{
    std::string ExtraMessage = fmt::format("POST_PROCESS_FINISH({},{})", iProjectName, iTestName);
    EncodeText(ExtraMessage, TCPGRAM_CODE_EVENT);
}

bool CTCPGram::GetCommand(std::string &rString)
{
    unsigned char Code = GetCode();

    if (Code != TCPGRAM_CODE_COMMAND)
        return false;

    rString = ToLowerCase(GetText());
    return true;
}

CTCPGram::CTCPGram(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    if (ReturnTCPGram)
    {
        m_MessageHeader = ReturnTCPGram->m_MessageHeader;
        m_Destination   = ReturnTCPGram->m_Destination;
        m_Data.swap(ReturnTCPGram->m_Data);
    }
}

#endif

void CTCPGram::Clear()
{
    m_Data.clear();
    m_MessageHeader.Reset();
}

std::exception CTCPGram::GetException()
{
    std::string ErrorMessage;
    if (GetCode() == TCPGRAM_CODE_ERROR)
    {
        ErrorMessage = GetText();
        return std::exception(ErrorMessage.c_str());
    }
    return std::exception("No error");
}
