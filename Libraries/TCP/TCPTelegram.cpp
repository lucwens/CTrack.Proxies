
#ifdef CTRACK
#include "stdafx.h"
#endif

#include "TCPTelegram.h"
#include "../XML/TinyXML_AttributeValues.h"
#include "../Utility/Print.h"

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

void CTCPGram::EncodeDoubleArray(std::vector<double> &iDoubleArray, unsigned char Code, bool bCopyArray, bool DoNotSendHeader)
{
    auto NumChannels = iDoubleArray.size();
    if (bCopyArray)
    {
        std::vector<double> arDoubles(iDoubleArray);
        m_Data.swap(*reinterpret_cast<std::vector<char> *>(&arDoubles));
    }
    else
    {
        m_Data.swap(*reinterpret_cast<std::vector<char> *>(&iDoubleArray));
    }
    SetCode(Code);
    SetPayloadSize(sizeof(double) * NumChannels);
}

CTCPGram::CTCPGram(TiXmlElement &rCommand, unsigned char Code)
{
    std::string XMLText = XMLToString(&rCommand);
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(std::vector<double> &arDoubles)
{
    // we need to make a copy of the doubles here because we assume that the arDoubles will be re-used to set new data
    size_t PackageSize = static_cast<size_t>(sizeof(double) * arDoubles.size());
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(TCPGRAM_CODE_DOUBLES);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), arDoubles.data(), PackageSize);
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
    m_Data.assign(string.begin(), string.end());
    size_t PackageSize = m_Data.size();
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
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

#ifdef _MANAGED

CTCPGram::CTCPGram(cliext::vector<double> arDoubles)
{
    size_t NumDoubles  = arDoubles.size();
    size_t PackageSize = static_cast<size_t>(sizeof(double) * NumDoubles);
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(TCPGRAM_CODE_DOUBLES);
    m_Data.resize(PackageSize);

    double *pDouble = reinterpret_cast<double *>(m_Data.data());
    for (size_t c = 0; c < NumDoubles; c++)
        pDouble[c] = arDoubles[c];
}

#endif

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

size_t CTCPGram::GetSize()
{
    return m_MessageHeader.GetPayloadSize();
}

const char *CTCPGram::GetText()
{
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS)
        return NULL;
    return reinterpret_cast<const char *>(m_Data.data());
}

std::vector<char> CTCPGram::GetData()
{
    return std::move(m_Data);
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

bool CTCPGram::GetString(std::string &rString)
{
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS)
        return false;
    rString = GetText();
    return true;
}

bool CTCPGram::GetDoubleQue(std::deque<double> &queDoubles)
{
    unsigned char Code = GetCode();

    if (Code != TCPGRAM_CODE_DOUBLES)
        return false;

    queDoubles.clear();
    std::vector<double> arDoubles;
    arDoubles.swap(*reinterpret_cast<std::vector<double> *>(&m_Data));
    for (auto &dValue : arDoubles)
        queDoubles.emplace_back(dValue);
    return true;
}

bool CTCPGram::GetDoubleArray(std::vector<double> &arDoubles)
{
    unsigned char Code = GetCode();

    if (Code != TCPGRAM_CODE_DOUBLES)
        return false;
    arDoubles.clear();
    arDoubles.swap(*reinterpret_cast<std::vector<double> *>(&m_Data));
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

CTCPGram::CTCPGram(HMatrix *phMatix, ChartIndex iIndex)
{
    int NumChannels = phMatix->Cols();
    m_Destination   = ALL_DESTINATIONS;
    m_Source        = 0;

    std::vector<double> arDoubles(NumChannels);
    for (int c = 0; c < NumChannels; c++)
        arDoubles[c] = (*phMatix)(iIndex, c);
    SetPayloadSize(sizeof(double) * NumChannels);
    m_Data.swap(*reinterpret_cast<std::vector<char> *>(&arDoubles));
}

bool CTCPGram::GetMm(Mm &rMatrix)
{
    unsigned char Code = GetCode();

    if (Code != TCPGRAM_CODE_DOUBLES)
        return false;

    std::vector<double> arDoubles;
    arDoubles.swap(*reinterpret_cast<std::vector<double> *>(&m_Data));
    size_t NumDoubles = arDoubles.size();
    for (int c = 0; c < NumDoubles; c++)
        rMatrix.r(1, c + 1) = arDoubles[c];

    if (NumDoubles != rMatrix.cols() && rMatrix.rows() != 1)
        rMatrix = zeros(1, NumDoubles);

    return true;
}
#endif

void CTCPGram::Clear()
{
    m_Data.clear();
    m_MessageHeader.Reset();
}
