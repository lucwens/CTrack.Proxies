
#include "TCPTelegram.h"
#include "../XML/TinyXML_AttributeValues.h"
#include "../Utility/Print.h"

//------------------------------------------------------------------------------------------------------------------
/*
CTCPGRam class
*/
//------------------------------------------------------------------------------------------------------------------

CTCPGram::CTCPGram(T_MessageHeader &messageHeader, std::vector<char> &dataBuffer)
{
    m_Destination   = ALL_DESTINATIONS;
    m_MessageHeader = messageHeader;
    m_Data          = std::move(dataBuffer);
    messageHeader.Reset();
}

CTCPGram::CTCPGram(char *pBytes, size_t PackageSize, unsigned char Code)
{
    m_Destination = ALL_DESTINATIONS;
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), pBytes, PackageSize);
}

void CTCPGram::EncodeText(const std::string &iText, unsigned char Code)
{
    m_Destination      = ALL_DESTINATIONS;
    size_t PackageSize = static_cast<unsigned long>(sizeof(char) * (iText.size() + 1));
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), iText.c_str(), PackageSize);
}

CTCPGram::CTCPGram(TiXmlElement &rCommand, unsigned char Code)
{
    std::string XMLText = XMLToString(&rCommand);
    EncodeText(XMLText, Code);
}

CTCPGram::CTCPGram(std::vector<double> &arDoubles)
{
    // we need to make a copy of the doubles here because we assume that the arDoubles will be re-used to set new data
    m_Destination      = ALL_DESTINATIONS;
    size_t PackageSize = static_cast<size_t>(sizeof(double) * arDoubles.size());
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(TCPGRAM_CODE_DOUBLES);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), arDoubles.data(), PackageSize);
}

CTCPGram::CTCPGram(std::vector<char> &arBytes, unsigned char Code)
{
    m_Destination      = ALL_DESTINATIONS;
    size_t PackageSize = arBytes.size();
    m_MessageHeader.SetPayloadSize(PackageSize);
    m_MessageHeader.SetCode(Code);
    m_Data.resize(PackageSize);
    memcpy(m_Data.data(), arBytes.data(), PackageSize);
}

CTCPGram::CTCPGram(std::vector<char> &&arBytes, unsigned char Code)
{
    m_Destination      = ALL_DESTINATIONS;
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
    m_Destination      = ALL_DESTINATIONS;
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

void CTCPGram::Clear()
{
    m_Data.clear();
    m_MessageHeader.Reset();
}
