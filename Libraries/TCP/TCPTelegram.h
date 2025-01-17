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

// in the CTCPGram the next bytes are used
constexpr unsigned char TCPGRAM_INDEX_SIZE      = 0;
constexpr unsigned char TCPGRAM_INDEX_CODE      = 4;
constexpr unsigned char TCPGRAM_INDEX_PAYLOAD   = 5;
constexpr unsigned char TCPGRAM_HEADER_SIZE     = 5;
constexpr unsigned char HEADER_SIZE             = 4;

// Codes for tcpgrams
constexpr unsigned char TCPGRAM_CODE_INVALID    = 0;         // invalid return
constexpr unsigned char TCPGRAM_CODE_DOUBLES    = 1;         // array of doubles
constexpr unsigned char TCPGRAM_CODE_COMMAND    = 2;         // xml containing command
constexpr unsigned char TCPGRAM_CODE_STATUS     = 3;         // xml containing status
constexpr unsigned char TCPGRAM_CODE_TEST_BIG   = 4;         // xml containing status
constexpr unsigned char TCPGRAM_CODE_DONT_USE   = UCHAR_MAX; // contains a warning

constexpr int  ALL_DESTINATIONS                 = 0;
//--------------------------------------------------------------------------------------------------------------------------------------
/*
Packing and unpacking commands to/from XML
*/
//--------------------------------------------------------------------------------------------------------------------------------------
constexpr char TCP_XML_TAG_COMMAND[]            = "COMMAND";
constexpr char TCP_XML_ATTRIB_COMMAND[]         = "command";
constexpr char TCP_XML_ATTRIB_RESULT[]          = "result";
constexpr char TCP_XML_ATTRIB_COMMAND_DETECT[]  = "detect";  // => sends xml information back about the possible configurations and the detected configuration
constexpr char TCP_XML_ATTRIB_COMMAND_EDIT[]    = "edit";    // => launches the settings dialog for the scanner, can be used to start the qualification
constexpr char TCP_XML_ATTRIB_COMMAND_CONNECT[] = "connect"; // => starts scanning, parameters are desired localizer and scanner
constexpr char TCP_XML_ATTRIB_COMMAND_CONNECT_LOCALIZER[] = "localizer";  // => localizer
constexpr char TCP_XML_ATTRIB_COMMAND_CONNECT_SCANNER[]   = "scanner";    // => scanner
constexpr char TCP_XML_ATTRIB_COMMAND_SCAN_START[]        = "scan_start"; // => starts scanning, no parameters
constexpr char TCP_XML_ATTRIB_COMMAND_SCAN_STOP[]         = "scan_stop";  // => stops scanning, no parameters
constexpr char TCP_XML_ATTRIB_COMMAND_DISCONNECT[]        = "disconnect"; // => disconnects, no parameters
constexpr char TCP_XML_ATTRIB_COMMAND_EXIT[]              = "exit";
constexpr char TCP_XML_ATTRIB_COMMAND_TEST_BIG[]          = "big";

constexpr char TCP_XML_TAG_STATUS[]                       = "STATUS";
constexpr char TCP_XML_TAG_STATUS_ATTRIB[]                = "status";
constexpr char TCP_XML_TAG_STATUS_IDLE[]                  = "idle";
constexpr char TCP_XML_TAG_STATUS_CONNECTED[]             = "connected";
constexpr char TCP_XML_TAG_STATUS_SCANNING[]              = "scanning";
constexpr char TCP_XML_TAG_STATUS_ERROR[]                 = "error : %s";

// creation of command XML
std::unique_ptr<TiXmlElement> CreateCommandDetect();
std::unique_ptr<TiXmlElement> CreateCommandEdit();
std::unique_ptr<TiXmlElement> CreateCommandConnect(std::string &iLocalizerName, std::string &iScannerName);
std::unique_ptr<TiXmlElement> CreateCommandScanStart();
std::unique_ptr<TiXmlElement> CreateCommandScanStop();
std::unique_ptr<TiXmlElement> CreateCommandDisconnect();
std::unique_ptr<TiXmlElement> CreateCommandExit();
std::unique_ptr<TiXmlElement> CreateCommandTestBigPacket(unsigned long NumLongs = 80000);

// decoding command XML's
bool          DecodeCommand(std::unique_ptr<TiXmlElement> &pXML, std::string &Command, std::map<std::string, std::string> &rParameters);
unsigned long GetSize(std::vector<std::uint8_t> &Buffer);

// class CTCPGram can contain status,commands,  data
class CTCPGram
{
    friend class CSocket;

  public: // make movable only
    CTCPGram(CTCPGram &&rTCPGram) noexcept
    {
        m_Data = std::move(rTCPGram.m_Data);
    };
    CTCPGram &operator=(CTCPGram &&rTCPGram) noexcept
    {
        m_Data = std::move(rTCPGram.m_Data);
        return *this;
    }
    CTCPGram(CTCPGram &rTCPGram)            = delete;
    CTCPGram &operator=(CTCPGram &rTCPGram) = delete;

  public: // various
    explicit CTCPGram() = default;
    explicit CTCPGram(char *pBytes, unsigned long NumBytes, unsigned char Code);
    explicit CTCPGram(TiXmlElement &rCommand, unsigned char Code);
    explicit CTCPGram(std::unique_ptr<TiXmlElement> &rCommand, unsigned char Code);
    explicit CTCPGram(std::vector<double> &arDoubles);
    explicit CTCPGram(std::vector<std::uint8_t> &arBytes, unsigned char Code);
    explicit CTCPGram(std::vector<std::uint8_t> &arBytes);
#ifdef _MANAGED
    CTCPGram(cliext::vector<double>  arDoubles);
#endif
    virtual ~CTCPGram()
    {
        Clear();
    }

  public:
    void SetDestination(SOCKET iDestination)
    {
        m_Destination = iDestination;
    };
    SOCKET GetDestination()
    {
        return m_Destination;
    };
    void CopyFrom(std::unique_ptr<CTCPGram> &);

  public:
    virtual void                          EncodeText(const std::string &iText, unsigned char Code);
    virtual unsigned char                 GetCode();
    virtual unsigned long                 GetSize();
    const char                           *GetText();
    virtual std::unique_ptr<TiXmlElement> GetXML(); // returned pointer must be deleted by receiving code
    virtual void                          Clear();

  public:
    std::vector<std::uint8_t> m_Data;
    SOCKET                    m_Destination = 0; // if 0 then all client sockets will get this telegram
};
