#pragma once

#include "TCPTelegram.h"
#include <list>
#include <memory>
#include <set>
#include <atomic>
#include <thread>
#include <functional>
#include <mutex>

using namespace std;

#define DEFAULT_TCP_PORT 40000
#define DEFAULT_TCP_HOST ("localhost")

void InterruptSet(bool bValue = true);
//------------------------------------------------------------------------------------------------------------------
/*
Ping : check if a destination can be reached
*/
//------------------------------------------------------------------------------------------------------------------

bool Ping(const char *ipAddress, std::string &FeedBack, DWORD iTimeout = 1000);

//--------------------------------------------------------------------------------------------------------------------------------------
/*
Resolve Ip strings like "127.0.0.1", "\\mycomputer", "www.test.com" to an IP address
*/
//--------------------------------------------------------------------------------------------------------------------------------------
bool ResolveIP4_Address(const std::string HostName, std::string &IP_Number); // returns true on succes

typedef std::function<void()> StateResponder;   // function for state entry/run/exit
typedef std::function<void()> CommandResponder; // function for CNode responders
typedef std::function<void(size_t)>
    ConnectResponder; // function when the number of connections changed, new number of connections is passed as integer to function

//------------------------------------------------------------------------------------------------------------------
/*
The communication class permits to send messages to and from between engine and UI.
This class supports:
- TCP server
- TCP client
- UDP

Messages are fed via two FIFO buffers, one for reading, one for writing.
The actual socket communication is done via a dedicated thread.


The TCP server supports multiple clients.
Send packages will be send to all clients from the SendBuffer.
Received packages are all assembled in the ReceiveiBuffer in order of complete reception, so when the
last byte is in, the packet is shifted into the receivebuffer and becomes available to the main program.

This class also supports large packets, that is for TCP, not UDP of course. For UDP the maximum allowed telegram
size is checked, and this maximum size is checked on every transmission.
For reception, each socket has its own assembly buffer, where every reception with receive is stored, appended to
an internal buffer. When the buffer is bigger than the individual telegram (determined by the telegram that is in the
header), then the telegram is transferred to the ReceiveBuffer and becoming available to the main program.

For sending a similar mechanism is used: the telegrams in the sendbuffer can be send in pieces. Only when all pieces
of the telegram have been send over all sockets, the telegram is removed from the sendbuffer.
This means that all sockets will only start getting parts of the next message when the current message is transmitted
completely.

*/
//------------------------------------------------------------------------------------------------------------------

enum E_COMMUNICATION_Mode
{
    TCP_SERVER,
    UDP,
    TCP_CLIENT
};

//------------------------------------------------------------------------------------------------------------------
/*
CSocket represents a single network connection and is responsible for extracting telegrams with ReadExtractTelegram
The default extraction is based on the 4-byte size at the start of CTCPGram, if the telegrams are separated in
a different way(e.g. delimiter at the end), then the function ReadExtractTelegram needs to be overridden
*/
//------------------------------------------------------------------------------------------------------------------
class CSocket
{
  public:
    CSocket(SOCKET iSocket, E_COMMUNICATION_Mode, SOCKADDR_IN *ipSockAddress, int iUDPReceivePort, bool iUDPBroadcast, const std::string iUDPSendToAddress);
    virtual ~CSocket();
    SOCKET GetSocket()
    {
        return m_Socket;
    };

  public: // socket manipulations
    void SetNagle(bool bDisableNagle = true);
    void SetNonBlocking(bool bNonBlocking = true);
    void SetReuseAddress(bool bEnableReuseAddress = true);
    void SetBroadcast(bool bEnableBroadcast = true);
    int  GetMaxUDPMessageSize();
    void SetBlockWrite(bool ibBlockWrite = true)
    {
        m_bBlockWrite = ibBlockWrite;
    };

  public:
    virtual bool DataAvailable();  // returns true if data is available for reading, throws FALSE if connection was reset or CExceptionSocket for socket error
    virtual void ReadFeedBuffer(); // appends read data, produces a CTCPGram when the complete package is received, throws FALSE if connection was reset or
                                   // CExceptionSocket for socket error
    virtual bool ReadExtractTelegram(std::unique_ptr<CTCPGram> &); // extracts the oldest telegram from the internal buffer
    virtual void WriteSendReset();                                 // resets the write buffer index at the start of a new telegram
    virtual bool WriteSend(std::unique_ptr<CTCPGram> &); // continues writing packets of the CTCPGram, when the complete TCPGram has been transmitted, then true
                                                         // is returned, throws FALSE if connection was reset or CExceptionSocket for socket error
  protected:                                             // manipulation of internal buffers
    virtual void                      AppendBack(std::vector<std::uint8_t> &mainBuffer, const std::vector<std::uint8_t> &appendBuffer);
    virtual std::vector<std::uint8_t> RemoveFront(std::vector<std::uint8_t> &mainBuffer, unsigned long sizeToRemove);

  protected: // socket and related
    SOCKET               m_Socket;
    E_COMMUNICATION_Mode m_CommunicationMode = TCP_SERVER;
    SOCKADDR_IN         *m_pSockAddress      = nullptr;
    struct sockaddr_in   m_UDPBroadCastAddr;

  protected: // read buffer
    unsigned long m_CurrentTelegramSize =
        0; // the size of a telegram is in the front of a message, that is set here when a new message is started for reception
    std::vector<std::uint8_t> m_AccumulationBuffer; // buffer containing accumulated contents of the telegram
  protected:                                        // write buffer
    unsigned long m_TotalBytesWritten = 0;          // number of bytes written so far for the current TCPGram
    unsigned long m_MaxUDPMessageSize = 0;          // only useful for UDP to generate an exception when the datagram is bigger than allowed
    bool m_bBlockWrite = false; // if true then the socket will not write, this is used so that we can send a configuration first before sending anything else
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationInterface : class holding parameters, common parent for CCommunicationTCP and CCommunicationThread
This class also contains lists for incoming and outgoing telegrams and a list for newcomers. Newcomers are new client
sockets that connect to a server socket. The newcomer list allows to send a welcome message or take other actions when a
new client connects to a server.
All data members have thread safe access (m_Lock)
*/
//------------------------------------------------------------------------------------------------------------------
class CCommunicationInterface
{
    friend class CCommunicationThread;

  public:
    void CopyFrom(CCommunicationInterface *ipFrom);

  public:
    E_COMMUNICATION_Mode GetCommunicationMode();
    void                 SetCommunicationMode(E_COMMUNICATION_Mode iMode);
    unsigned short       GetPort();
    unsigned short       GetPortUDP();
    bool                 GetUDPBroadcast();
    std::string          GetHostName();
    bool                 IsServer();
    bool                 ErrorOccurred();
    virtual std::string  GetError();
    virtual void         SetError(const std::string iFileName, int iLineNumber, const std::string iMessage);

  public:                           // CCommunicationParameters
    virtual CSocket *GetNewComer(); // pops socket of oldest client that recently connected to our server, returns INVALID_SOCKET if no new sockets are
                                    // available
    virtual void     AddNewComer(CSocket *);
    virtual size_t   GetNumConnections()
    {
        return 0;
    };

  public: // sending data : pushes a telegram into the FIFO stack
    virtual bool GetSendPackage(
        std::unique_ptr<CTCPGram> &); // thrd: pops the oldest package from the start of the list, the returned CTCPGram needs to be destroyed afterwards
    virtual bool GetReceivePackage(std::unique_ptr<CTCPGram> &,
                                   unsigned char Code = TCPGRAM_CODE_DONT_USE); // Object: pops the oldest package from the start of the list, the returned
                                                                                // CTCPGram needs to be destroyed afterwards
    virtual bool GetLastReceivePackage(std::unique_ptr<CTCPGram> &);            // only last, most recent package, others are deleted
    virtual void PushSendPackage(std::unique_ptr<CTCPGram> &);                  // Object: pushes a new send package to the end of the list
    virtual void PushReceivePackage(std::unique_ptr<CTCPGram> &);               // thrd: pushes a new received package to the end of the list
    virtual void ClearBuffers();
    virtual void RemoveOldReceiveTelegrams(int iNumberToKeep); // removes old packages, keeps iNumberToKeep
  protected:                                                   // sockets and states
    E_COMMUNICATION_Mode m_CommunicationMode = TCP_SERVER;
    unsigned short       m_Port              = 50000; // default 50000
    unsigned short       m_PortUDP           = 50002; // default 50000
    std::string          m_HostName;                  // hostname, like "ctech-tower", "127.0.0.1" "localhost" "www.ctechmetrology.com"
    bool                 m_bUDPBroadCast = true;      // if false, m_IP4Address is used to send to
    bool                 m_bMakeBlocking = false;     // makes the socket blocking
    bool                 m_bEnableNagle  = false;     // enables Nagle grouping of blocks
    float                m_TimeOut       = 0.5;       // time-out in seconds, only when blocking is activated
  protected:                                          // error handling
    bool              m_bErrorOccurred = false;       // set to true when an error occurred, further information in m_ErrorString
    std::string       m_ErrorString;                  // error that caused our communication to stops
    std::string       m_ErrorSourceFile;
    int               m_ErrorSourceLine = 0;
    std::atomic<bool> m_bInitialized    = false;

  protected: // buffers
    std::list<std::unique_ptr<CTCPGram>> m_arSendBuffer;
    std::list<std::unique_ptr<CTCPGram>> m_arReceiveBuffer;
    std::list<CSocket *> m_arNewComers; // list of sockets that newly connected to our server socket, used to send a configuration to the newly connected socket
                                        // when the engine is running
    std::recursive_mutex m_Mutex;       // critical section to be used with CSingleLock to protect data
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationTCP manages 1 (UDP,Client) or multiple sockets(SERVER)
It is also responsible for converting incoming data from the sockets to telegrams
So if a different protocol requires a different way of doing this, derive from this and override
*/
//------------------------------------------------------------------------------------------------------------------
class CCommunicationThread;

class CCommunicationObject : public CCommunicationInterface
{
  public:
    CCommunicationObject();
    virtual ~CCommunicationObject();

  public: // open close state
    virtual void Open(E_COMMUNICATION_Mode iTcpMode = TCP_SERVER, int iPort = 40000, int iPortUDP = 0, const std::string iIpAddress = ("127.0.0.1"));
    virtual void Close();

  public: // from statemanager
    virtual void SetError(const std::string iFileName, int iLineNumber, LPCTSTR iMessage)
    {
        ;
    };
    virtual void SetError(const std::string iFileName, int iLineNumber, const std::string iMessage)
    {
        ;
    };
    bool IsConnected()
    {
        return m_TCPNumConnections > 0;
    };
    std::string GetHost()
    {
        return GetHostName();
    };
    void         CheckConnections();
    virtual void TCPReceiveRespond();
    void         Run();
    void         IdleRun();

  public: // to be used in CNode derivations
    void XML_ReadWrite(TiXmlElement *&, bool Read /* XML_WRITE XML_READ */);
    void CopyFrom(CCommunicationObject *);

  public: // CCommunicationParameters
    size_t GetNumConnections() override;
    void   PushSendPackage(std::unique_ptr<CTCPGram> &) override;

  public: // own overrideable functions
    virtual CSocket *SocketCreate(SOCKET iSocket, E_COMMUNICATION_Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPReceivePort, bool UDPBroadcast,
                                  const std::string UDPSendPort); // CSocket* ipSocket,bool bAddToNewComerList = false);public: // get set ip4 related stuff

  protected:
    std::shared_ptr<CCommunicationThread> m_pCommunicationThread;
    std::mutex  m_LockThreadRunning; // critical section owned by CCommunicationThread during its life, used to check when the thread has finished
    std::string m_ThreadName;

  protected: // added from statemanager
    size_t           m_TCPNumConnections{0};
    ConnectResponder m_TCPConnectChange;
    bool             m_bTCPServer{true};
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationThread manages the communication thread. We keep one communication thread for one port, so if
multiple CCommunicationTCP objects want to communicate through the same port, they have to work with the same thread
*/
//------------------------------------------------------------------------------------------------------------------

class CCommunicationThread : public CCommunicationInterface
{
    friend UINT CommunicationThread(LPVOID lpParameter);

  public:
    CCommunicationThread();
    virtual ~CCommunicationThread();
    void   CommunicationObjectAdd(CCommunicationObject &);
    void   CommunicationObjectRemove(CCommunicationObject &);
    size_t CommunicationObjectGetNum();

  public: // CCommunicationParameters
    void   AddNewComer(CSocket *) override;
    size_t GetNumConnections() override;
    void   PushReceivePackage(std::unique_ptr<CTCPGram> &) override;
    void   SetError(const std::string iFileName, int iLineNumber, const std::string iMessage) override;

  protected:
    void SocketAdd(SOCKET iSocket, E_COMMUNICATION_Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPReceivePort, bool bAddToNewComerList, bool UDPBroadcast,
                   const std::string UDPSendPort); // CSocket* ipSocket,bool bAddToNewComerList = false);
    CSocket *SocketFirst();                        // first in list, or NULL
    CSocket *SocketNext();                         // next, can only be called after SocketFirst
    CSocket *SocketDeleteCurrent();                // deletes current socket and return pointer to next socket
    void     SocketDeleteAll();                    // deletes all sockets
    void     SocketResetSend();                    // resets the write pointers of all sockets
  public:                                          // thread management
    void ThreadFunction();
    void StartThread();
    void EndThread();
    void SetQuit(bool ibQuit = true);
    bool GetQuit();

  protected: // thread members
    std::thread                      m_Thread;
    std::atomic<bool>                m_bQuit       = false;
    std::atomic<bool>                m_bIntialized = false;
    std::list<CSocket *>::iterator   m_IterCurrentSocket;
    std::list<CSocket *>             m_arSockets;
    std::set<CCommunicationObject *> m_setCommunicationObject;
    std::string                      m_ThreadName;
};
