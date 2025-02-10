#pragma once

#include "../XML/TinyXML_Extra.h"
#include <deque>
#include <winsock2.h>
#include <memory>
#include <string>

#ifdef _MANAGED
#include <cliext/vector>
#endif
//--------------------------------------------------------------------------------------------------------------------------------------
/*
TCPGram data packet represents the telegram. The typical use is that you derive from CTCPGram and provided a constructor for
each type of data you want to send, the constructor is responsible for converting your data to the binary telegram
*/
//--------------------------------------------------------------------------------------------------------------------------------------

// Codes for tcpgrams
constexpr unsigned char TCPGRAM_CODE_INVALID  = 0;         // invalid return
constexpr unsigned char TCPGRAM_CODE_DOUBLES  = 1;         // array of doubles
constexpr unsigned char TCPGRAM_CODE_COMMAND  = 2;         // xml containing command
constexpr unsigned char TCPGRAM_CODE_STATUS   = 3;         // xml containing status
constexpr unsigned char TCPGRAM_CODE_TEST_BIG = 4;         // xml containing status
constexpr unsigned char TCPGRAM_CODE_DONT_USE = UCHAR_MAX; // contains a warning

constexpr int ALL_DESTINATIONS                = 0;

//--------------------------------------------------------------------------------------------------------------------------------------
//
// Receive header
//
//--------------------------------------------------------------------------------------------------------------------------------------

struct T_MessageHeader
{
  public:
    size_t        GetHeaderSize() { return sizeof(T_MessageHeader); }
    size_t        GetPayloadSize() { return m_Size - sizeof(T_MessageHeader); }
    unsigned char GetCode() { return m_Code; }
    void          SetCode(unsigned char iCode) { m_Code = iCode; }
    void          SetPayloadSize(size_t iSize) { m_Size = iSize + sizeof(T_MessageHeader); }
    void          Reset()
    {
        m_Size = 0;
        m_Code = 0;
    };

  protected:
    size_t        m_Size{0}; // size includes the size of the header and the payload, legacy reasons
    unsigned char m_Code;
};

// decoding command XML's
bool DecodeCommand(std::unique_ptr<TiXmlElement> &pXML, std::string &Command, std::map<std::string, std::string> &rParameters);

// class CTCPGram can contain status,commands,  data
class CTCPGram
{
    friend class CSocket;

  public: // make movable only
    CTCPGram(CTCPGram &&rTCPGram) noexcept { m_Data = std::move(rTCPGram.m_Data); };
    CTCPGram &operator=(CTCPGram &&rTCPGram) noexcept
    {
        m_MessageHeader = rTCPGram.m_MessageHeader;
        m_Data          = std::move(rTCPGram.m_Data);
        rTCPGram.m_MessageHeader.Reset();
        return *this;
    }
    CTCPGram(CTCPGram &rTCPGram)            = delete;
    CTCPGram &operator=(CTCPGram &rTCPGram) = delete;

  public: // various
    explicit CTCPGram() = default;
    explicit CTCPGram(T_MessageHeader &messageHeader, std::vector<char> &dataBuffer); // moves dataBuffer into m_Data
    explicit CTCPGram(char *pBytes, size_t NumBytes, unsigned char Code);
    explicit CTCPGram(TiXmlElement &rCommand, unsigned char Code);
    explicit CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code);
    explicit CTCPGram(std::vector<double> &arDoubles);
    explicit CTCPGram(std::vector<char> &arBytes, unsigned char Code);
#ifdef _MANAGED
    CTCPGram(cliext::vector<double> arDoubles);
#endif
    virtual ~CTCPGram() { Clear(); }

  public:
    void   SetDestination(SOCKET iDestination) { m_Destination = iDestination; };
    SOCKET GetDestination() { return m_Destination; };
    void   CopyFrom(std::unique_ptr<CTCPGram> &);

  public:
    virtual void                          EncodeText(const std::string &iText, unsigned char Code);
    virtual unsigned char                 GetCode();
    virtual size_t                        GetSize();
    const char                           *GetText();
    virtual std::unique_ptr<TiXmlElement> GetXML(); // returned pointer must be deleted by receiving code
    virtual void                          Clear();

  public:
    T_MessageHeader   m_MessageHeader;
    std::vector<char> m_Data;
    SOCKET            m_Destination = 0; // if 0 then all client sockets will get this telegram
};
