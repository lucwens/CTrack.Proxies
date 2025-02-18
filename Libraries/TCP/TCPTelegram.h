#pragma once

#include "..\..\Libraries\XML\tinyxml.h"
#include <deque>
#include <winsock2.h>
#include <memory>
#include <string>
#include <vector>

#ifdef _MANAGED
#include <cliext/vector>
#endif

#ifdef CTRACK
#include "Matrix.h"
#include "XML.h"
#endif
//--------------------------------------------------------------------------------------------------------------------------------------
/*
TCPGram data packet represents the telegram. The typical use is that you derive from CTCPGram and provided a constructor for
each type of data you want to send, the constructor is responsible for converting your data to the binary telegram
*/
//--------------------------------------------------------------------------------------------------------------------------------------

// Codes for tcpgrams
constexpr unsigned char TCPGRAM_CODE_INVALID   = 0;         // invalid return
constexpr unsigned char TCPGRAM_CODE_DOUBLES   = 1;         // array of doubles
constexpr unsigned char TCPGRAM_CODE_COMMAND   = 2;         // xml containing command
constexpr unsigned char TCPGRAM_CODE_STATUS    = 3;         // xml containing status
constexpr unsigned char TCPGRAM_CODE_STRING    = 4;         // string
constexpr unsigned char TCPGRAM_CODE_INTERRUPT = 4;         // string
constexpr unsigned char TCPGRAM_CODE_WARNING   = 6;         // contains a warning
constexpr unsigned char TCPGRAM_CODE_TEST_BIG  = 10;        // xml containing status
constexpr unsigned char TCPGRAM_CODE_DONT_USE  = UCHAR_MAX; // contains a warning

constexpr int ALL_DESTINATIONS                 = 0;

//--------------------------------------------------------------------------------------------------------------------------------------
//
// Receive header
//
//--------------------------------------------------------------------------------------------------------------------------------------

__pragma(pack(push, 1)) struct TMessageHeader
{
  public:
    size_t        GetHeaderSize() { return sizeof(TMessageHeader); }
    size_t        GetPayloadSize() { return m_Size - sizeof(TMessageHeader); }
    unsigned char GetCode() { return m_Code; }
    char         *GetData() { return reinterpret_cast<char *>(this); }
    void          SetCode(unsigned char iCode) { m_Code = iCode; }
    void          SetPayloadSize(size_t iSize) { m_Size = iSize + sizeof(TMessageHeader); }
    void          Reset()
    {
        m_Size = 0;
        m_Code = 0;
    };

  protected:
    size_t        m_Size{0}; // size includes the size of the header and the payload, legacy reasons
    unsigned char m_Code;
};
__pragma(pack(pop))

    // class CTCPGram can contain status,commands,  data
    class CTCPGram
{
    friend class CSocket;

  public: // various
    explicit CTCPGram() = default;
    explicit CTCPGram(TMessageHeader &messageHeader, std::vector<char> &dataBuffer); // moves dataBuffer into m_Data
    explicit CTCPGram(std::vector<char> &dataBuffer);                                 // telegrams without header
    explicit CTCPGram(char *pBytes, size_t NumBytes, unsigned char Code);
    explicit CTCPGram(TiXmlElement &rCommand, unsigned char Code);
    explicit CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code);
    explicit CTCPGram(std::vector<double> &arDoubles);

    // copy or move other std::vector<T>
    //
    // std::vector<unsigned long> arLong = {1,2,3};
    // std::vector<char> arChar;
    // arChar.swap(*reinterpret_cast<std::vector<char> *>(&arLong));
    // use this technique only with std::vector since that contains continous memory
    explicit CTCPGram(const std::vector<char> &arBytes, unsigned char Code); // makes a copy
    explicit CTCPGram(std::vector<char> &&arBytes, unsigned char Code);      // moves the data use std::move
    explicit CTCPGram(const std::string &arBytes, unsigned char Code);       // copy text
#ifdef _MANAGED
    CTCPGram(cliext::vector<double> arDoubles);
#endif
#ifdef CTRACK
    explicit CTCPGram::CTCPGram(CXML *ipXML, unsigned char Code);
    explicit CTCPGram(HMatrix *phMatix, ChartIndex iIndex);
    bool GetMm(Mm &rMatrix);
#endif
    virtual ~CTCPGram() { Clear(); }

  public: // make movable only
    explicit CTCPGram(CTCPGram &&rTCPGram) noexcept { m_Data = std::move(rTCPGram.m_Data); };
    CTCPGram &operator=(CTCPGram &&rTCPGram) noexcept
    {
        m_MessageHeader = rTCPGram.m_MessageHeader;
        m_Data          = std::move(rTCPGram.m_Data);
        rTCPGram.m_MessageHeader.Reset();
        return *this;
    }
    CTCPGram(CTCPGram &rTCPGram)            = delete;
    CTCPGram &operator=(CTCPGram &rTCPGram) = delete;
    void      CopyFrom(std::unique_ptr<CTCPGram> &);

  public:
    SOCKET GetDestination() { return m_Destination; };
    void   SetDestination(SOCKET iDestination) { m_Destination = iDestination; };
    void   SetSource(SOCKET iSource) { m_Source = iSource; };
    SOCKET GetSource() { return m_Source; };
    void   SetSendHeader(bool bSendHeader) { m_bSendHeader = true; };
    bool   GetSendHeader() { return m_bSendHeader; };

  public:
    virtual void          EncodeText(const std::string &iText, unsigned char Code);
    virtual void          EncodeDoubleArray(std::vector<double> &iDoubleArray, unsigned char Code, bool bCopyArray = true);
    virtual unsigned char GetCode();
    virtual void          SetCode(unsigned char iCode);
    virtual size_t        GetSize();
    size_t                GetPayloadSize() { return m_MessageHeader.GetPayloadSize(); }
    void                  SetPayloadSize(size_t iSize) { m_MessageHeader.SetPayloadSize(iSize); }
    const char           *GetText();
    std::vector<char>     GetData();
    virtual std::unique_ptr<TiXmlElement> GetXML(); // returned pointer must be deleted by receiving code
    virtual bool                          GetString(std::string &);
    virtual bool                          GetDoubleQue(std::deque<double> &queDoubles);
    virtual bool                          GetDoubleArray(std::vector<double> &arDoubles);
    virtual void                          Clear();

  public:
    TMessageHeader   m_MessageHeader;
    std::vector<char> m_Data;
    SOCKET            m_Destination = ALL_DESTINATIONS; // if 0 then all client sockets will get this telegram
    SOCKET            m_Source      = 0;
    bool              m_bSendHeader = true;
};
