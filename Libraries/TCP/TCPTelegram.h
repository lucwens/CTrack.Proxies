#pragma once

#include "Message.h"

#include <tinyxml.h>
#include <deque>
#include <winsock2.h>
#include <memory>
#include <string>
#include <vector>
#include <set>

#ifdef _MANAGED
#include <cliext/vector>
#endif

#ifdef CTRACK
#include "Matrix.h"
#include "XML.h"

class CState;
#endif
//--------------------------------------------------------------------------------------------------------------------------------------
/*
TCPGram data packet represents the telegram. The typical use is that you derive from CTCPGram and provided a constructor for
each type of data you want to send, the constructor is responsible for converting your data to the binary telegram
*/
//--------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------
// These numbers represent the type of telegrams send by CTrack client program
//------------------------------------------------------------------------------------------------------------------
// #define TCP_TYPE_DATA          0 // numerical data BYTE:NumChannels | DOUBLE:Channel 1 |....
// #define TCP_TYPE_COMMAND       1 // string containing information on the result of the executed command
// #define TCP_TYPE_STATUS        2 // string containing status information
// #define TCP_TYPE_CONFIGURATION 3 // ';' delimited string information containing channel names and units
// #define TCP_TYPE_STRING        4 // text string, e.g. to start/stop logging
// #define TCP_TYPE_EVENT         5 // string with feedback on the type of event that happened, an event occurs for example when the postprocess has finished
// and data has been exported to //       // disk

//==============================================================================
// TCP Message Codes
//==============================================================================
// The protocol uses only 2 message types:
// - TCPGRAM_CODE_DOUBLES (0): High-frequency binary measurement data
// - TCPGRAM_CODE_MESSAGE (8): JSON-based structured messages for everything else
//
// For CNode-derived objects, serialize to XML then embed as JSON payload:
//   { "id": "engine.command", "params": { "nodeType": "CConfiguration", "xml": "<Configuration>...</Configuration>" } }
//==============================================================================

// Primary codes (consolidated architecture)
constexpr unsigned char TCPGRAM_CODE_DOUBLES = 0; // Binary live measurement data (high-frequency doubles array)
constexpr unsigned char TCPGRAM_CODE_MESSAGE = 8; // JSON-based structured message (all control/status/commands)

// Backward compatibility alias
constexpr unsigned char TCPGRAM_CODE_DATA = TCPGRAM_CODE_DOUBLES;

// Legacy codes - DEPRECATED: Use TCPGRAM_CODE_MESSAGE with appropriate message ID instead
// These will be removed in a future version. Migration guide:
//   TCPGRAM_CODE_COMMAND       -> TCPGRAM_CODE_MESSAGE with id="engine.command"
//   TCPGRAM_CODE_STATUS        -> TCPGRAM_CODE_MESSAGE with id="engine.state"
//   TCPGRAM_CODE_CONFIGURATION -> TCPGRAM_CODE_MESSAGE with id="engine.config"
//   TCPGRAM_CODE_STRING        -> TCPGRAM_CODE_MESSAGE with id="engine.log"
//   TCPGRAM_CODE_EVENT         -> TCPGRAM_CODE_MESSAGE with id="engine.event"
//   TCPGRAM_CODE_INTERRUPT     -> TCPGRAM_CODE_MESSAGE with id="engine.interrupt"
//   TCPGRAM_CODE_ERROR         -> TCPGRAM_CODE_MESSAGE with id="engine.error"
// NOTE: [[deprecated]] attributes removed temporarily while device proxies are migrated
constexpr unsigned char TCPGRAM_CODE_COMMAND       = 1; // DEPRECATED - use engine.command
constexpr unsigned char TCPGRAM_CODE_STATUS        = 2; // DEPRECATED - use engine.state
constexpr unsigned char TCPGRAM_CODE_CONFIGURATION = 3; // DEPRECATED - use engine.config
constexpr unsigned char TCPGRAM_CODE_STRING        = 4; // DEPRECATED - use engine.log
constexpr unsigned char TCPGRAM_CODE_EVENT         = 5; // DEPRECATED - use engine.event
constexpr unsigned char TCPGRAM_CODE_INTERRUPT     = 6; // DEPRECATED - use engine.interrupt
constexpr unsigned char TCPGRAM_CODE_ERROR         = 7; // DEPRECATED - use engine.error

// Utility codes
constexpr unsigned char TCPGRAM_CODE_TEST_BIG = 10;        // test message with big payload
constexpr unsigned char TCPGRAM_CODE_INVALID  = 100;       // invalid return
constexpr unsigned char TCPGRAM_CODE_ALL      = UCHAR_MAX; // used in receive to indicate all messages

constexpr int ALL_DESTINATIONS                     = 0;

//--------------------------------------------------------------------------------------------------------------------------------------
//
// Receive header
//
//--------------------------------------------------------------------------------------------------------------------------------------

__pragma(pack(push, 1)) struct TMessageHeader
{
  public:
    std::uint32_t GetHeaderSize() { return sizeof(TMessageHeader); }
    std::uint32_t GetPayloadSize() { return m_Size - sizeof(TMessageHeader); }
    unsigned char GetCode() { return m_Code; }
    char         *GetData() { return reinterpret_cast<char *>(this); }
    void          SetCode(unsigned char iCode) { m_Code = iCode; }
    void          SetPayloadSize(std::uint32_t iSize) { m_Size = iSize + sizeof(TMessageHeader); }
    void          Reset()
    {
        m_Size = 0;
        m_Code = 0;
    };

  protected:
    std::uint32_t m_Size{0}; // size includes the size of the header and the payload, legacy reasons
    unsigned char m_Code{0};
};
__pragma(pack(pop))

    // class CTCPGram can contain status,commands,  data
    class CTCPGram
{
    friend class CSocket;

  public: // various
    explicit CTCPGram() = default;
    explicit CTCPGram(TMessageHeader &messageHeader, std::vector<char> &dataBuffer); // moves dataBuffer into m_Data
    explicit CTCPGram(std::vector<char> &dataBuffer);                                // telegrams without header
    explicit CTCPGram(char *pBytes, size_t NumBytes, unsigned char Code);
    explicit CTCPGram(TiXmlElement &rCommand, unsigned char Code);
    explicit CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code);
    explicit CTCPGram(std::vector<double> &arDoubles);
    explicit CTCPGram(const std::exception &);
    explicit CTCPGram(const CTrack::Message &);

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
    explicit CTCPGram(CXML *ipXML, unsigned char Code);
    explicit CTCPGram(HMatrix &rhInput, SOCKET iDestination);    // sends channel information
    explicit CTCPGram(ChartIndex FrameNumber, HMatrix &rhInput); // sends double data
    explicit CTCPGram(CState *pState);
    explicit CTCPGram(std::string &CommandReturn);
    explicit CTCPGram(const std::string &iProjectName, const std::string &iTestName);
    explicit CTCPGram(std::unique_ptr<CTCPGram> &ReturnTCPGram);
    virtual ~CTCPGram() = default;

  public:
    bool GetCommand(std::string &rString);
    bool GetMm(Mm &rMatrix);
#endif

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

  public:
    virtual void                          EncodeText(const std::string &iText, unsigned char Code);
    virtual void                          EncodeDoubleArray(std::vector<double> &iDoubleArray);
    virtual bool                          GetDoubleQue(std::deque<double> &queDoubles);
    virtual bool                          GetDoubleArray(std::vector<double> &arDoubles);
    virtual unsigned char                 GetCode();
    virtual void                          SetCode(unsigned char iCode);
    virtual std::uint32_t                 GetSize();
    std::uint32_t                         GetPayloadSize() { return m_MessageHeader.GetPayloadSize(); }
    void                                  SetPayloadSize(std::uint32_t iSize) { m_MessageHeader.SetPayloadSize(iSize); }
    std::string                           GetText();
    std::vector<char>                     GetData();
    virtual std::unique_ptr<TiXmlElement> GetXML(); // returned pointer must be deleted by receiving code
    virtual bool                          GetString(std::string &);
    bool                                  GetMessage(CTrack::Message &);
    virtual void                          Clear();
    virtual std::exception                GetException();

  public:
    TMessageHeader    m_MessageHeader;
    std::vector<char> m_Data;
    SOCKET            m_Destination = ALL_DESTINATIONS; // if 0 then all client sockets will get this telegram
    SOCKET            m_Source      = 0;
};
