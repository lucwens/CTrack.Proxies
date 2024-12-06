#pragma once

#include "TinyXML_Extra.h"
#include <deque>

//--------------------------------------------------------------------------------------------------------------------------------------
/*
TCPGram data packet represents the telegram. The typical use is that you derive from CTCPGram and provided a constructor for
each type of data you want to send, the constructor is responsible for converting your data to the binary telegram
*/
//--------------------------------------------------------------------------------------------------------------------------------------

// in the CTCPGram the next bytes are used
const unsigned char TCPGRAM_INDEX_SIZE(0);
const unsigned char TCPGRAM_INDEX_CODE(4);
const unsigned char TCPGRAM_INDEX_PAYLOAD(5);
const unsigned char TCPGRAM_HEADER_SIZE(5);
const unsigned char HEADER_SIZE(4);

// Codes for tcpgrams
const unsigned char TCPGRAM_CODE_INVALID(0);          // invalid return
const unsigned char TCPGRAM_CODE_DATA(1);             // array of doubles
const unsigned char TCPGRAM_CODE_COMMAND(2);          // xml containing command
const unsigned char TCPGRAM_CODE_STATUS(3);           // xml containing status
const unsigned char TCPGRAM_CODE_TEST_BIG(4);         // xml containing status
const unsigned char TCPGRAM_CODE_DONT_USE(UCHAR_MAX); // contains a warning

const int ALL_DESTINATIONS(0);
//--------------------------------------------------------------------------------------------------------------------------------------
/*
Packing and unpacking commands to/from XML
*/
//--------------------------------------------------------------------------------------------------------------------------------------
#define TCP_XML_TAG_COMMAND                      "COMMAND"
#define TCP_XML_ATTRIB_COMMAND                   "command"
#define TCP_XML_ATTRIB_RESULT                    "result"
#define TCP_XML_ATTRIB_COMMAND_DETECT            "detect"     // => sends xml information back about the possible configurations and the detected configuration
#define TCP_XML_ATTRIB_COMMAND_EDIT              "edit"       // => launches the settings dialog for the scanner, can be used to start the qualification
#define TCP_XML_ATTRIB_COMMAND_CONNECT           "connect"    // => starts scanning, parameters are desired localizer and scanner
#define TCP_XML_ATTRIB_COMMAND_CONNECT_LOCALIZER "localizer"  // => localizer
#define TCP_XML_ATTRIB_COMMAND_CONNECT_SCANNER   "scanner"    // => scanner
#define TCP_XML_ATTRIB_COMMAND_SCAN_START        "scan_start" // => starts scanning, no parameters
#define TCP_XML_ATTRIB_COMMAND_SCAN_STOP         "scan_stop"  // => stops scanning, no parameters
#define TCP_XML_ATTRIB_COMMAND_DISCONNECT        "disconnect" // => disconnects, no parameters
#define TCP_XML_ATTRIB_COMMAND_EXIT              "exit"
#define TCP_XML_ATTRIB_COMMAND_TEST_BIG          "big"

#define TCP_XML_TAG_STATUS                       "STATUS"
#define TCP_XML_TAG_STATUS_ATTRIB                "status"
#define TCP_XML_TAG_STATUS_IDLE                  "idle"
#define TCP_XML_TAG_STATUS_CONNECTED             "connected"
#define TCP_XML_TAG_STATUS_SCANNING              "scanning"
#define TCP_XML_TAG_STATUS_ERROR                 "error : %s"

// creation of command XML
std::unique_ptr<TiXmlElement> CreateCommandDetect();
std::unique_ptr<TiXmlElement> CreateCommandEdit();
std::unique_ptr<TiXmlElement> CreateCommandConnect(CStringA &iLocalizerName, CStringA &iScannerName);
std::unique_ptr<TiXmlElement> CreateCommandScanStart();
std::unique_ptr<TiXmlElement> CreateCommandScanStop();
std::unique_ptr<TiXmlElement> CreateCommandDisconnect();
std::unique_ptr<TiXmlElement> CreateCommandExit();
std::unique_ptr<TiXmlElement> CreateCommandTestBigPacket(unsigned long NumLongs = 80000);

// decoding command XML's
bool          DecodeCommand(std::unique_ptr<TiXmlElement> &pXML, CStringA &Command, std::map<CStringA, CStringA> &rParameters);
unsigned long GetSize(char *pBuffer);
void          SetSize(char *pBuffer, unsigned long iSize);
int           GetCode(char *pBuffer);
void          SetCode(char *pBuffer, int iCode);

// button states as coming from NMAPI
const int ButtonDown       = 0;
const int ButtonShortUp    = 1;
const int ButtonLongDown   = 2;
const int ButtonLongUp     = 3;
const int ButtonDoubleDown = 4;
const int ButtonDoubleUp   = 5;

//--------------------------------------------------------------------------------------------------------------------------------------
/*
Class for scanline
*/
//--------------------------------------------------------------------------------------------------------------------------------------

// 6DOF position of the localizer as a 4x4 matrix [axisX , axisY , AxisZ, position]
struct CycleItem6D
{
    double axisX[3];
    double axisY[3];
    double axisZ[3];
    double position[3];
    // unsigned long status;
};

// a scan line is an array of N ScanPoint3D, each point contains a x,y,z position, an intensity and a quality number, refer to "API Developer's Reference Manual.chm" of NMAPI for more information
struct ScanPoint3D
{
    double        position[3]; // x,y,z of point on the scan line
    double        intensity;
    double        quality;
    double        normal[3];
    unsigned long status = 0;
    // double drawRadius;
};

// these functions encode/decode a scanline into a buffer of bytes for sending/receiving a TCP telegram
unsigned long CalcBufferSize(unsigned long NScanPoints) noexcept;
void          EncodeScanToBuffer(char *pBuffer, size_t BufferSize, char &ButtonID, char &ButtonState, CycleItem6D &Pose, std::deque<ScanPoint3D> &ScanPoints) noexcept;
unsigned long DecodeScanFromBuffer(char *pBuffer, size_t BufferSize, char &ButtonID, char &ButtonState, CycleItem6D &Pose, std::deque<ScanPoint3D> &ScanPoints) noexcept;

// class CTCPGram can contain status,commands, linescan data
class CTCPGram
{
    friend class CSocket;

  public: // make movable only
    CTCPGram(CTCPGram &&rTCPGram)
    {
        m_pData                = std::move(rTCPGram.m_pData);
        m_PackageSize          = rTCPGram.m_PackageSize;
        rTCPGram.m_PackageSize = 0;
    };
    CTCPGram &operator=(CTCPGram &&rTCPGram)
    {
        m_pData                = std::move(rTCPGram.m_pData);
        m_PackageSize          = rTCPGram.m_PackageSize;
        rTCPGram.m_PackageSize = 0;
        return *this;
    }
    CTCPGram(CTCPGram &rTCPGram)            = delete;
    CTCPGram &operator=(CTCPGram &rTCPGram) = delete;

  public:                                      // various
    CTCPGram(char *pFromReadBuffer = nullptr); // constructor used by read routine
    CTCPGram(char *pBytes, unsigned long NumBytes, unsigned char Code);
    CTCPGram(std::unique_ptr<TiXmlElement> &ipNode, unsigned char Code);
    CTCPGram(TiXmlElement &rCommand, unsigned char Code);
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
    virtual void                          EncodeText(LPCSTR iText, unsigned char Code);
    virtual unsigned char                 GetCode();
    virtual unsigned long                 GetSize();
    const char                           *GetText();
    virtual std::unique_ptr<TiXmlElement> GetXML(); // returned pointer must be deleted by receiving code
    virtual void                          Clear();

  public:
    unsigned long         m_PackageSize = 0;
    std::unique_ptr<char> m_pData;       // not using a struct with a pointer to char for the data here, because we need a continuous block of data
    SOCKET                m_Destination; // if 0 then all client sockets will get this telegram
};
