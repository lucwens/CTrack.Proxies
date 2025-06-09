#pragma once

#ifdef CTRACK
#include "../Proxies/Libraries/TCP/TCPTelegram.h"
#include "../Proxies/Libraries/TCP/MessageResponder.h"
#include "../Proxies/Libraries/TCP/Subscriber.h"
#else
#include "TCPTelegram.h"
#include "MessageResponder.h"
#include "Subscriber.h"
#endif

#include <list>
#include <memory>
#include <vector>
#include <set>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

#ifdef CTRACK_UI
#include "DCommunication.h"
#endif

#ifdef CTRACK
class CNode;
class HMatrix;
#endif

#define STATEMANAGER_DEFAULT_TCP_PORT 40000
#define STATEMANAGER_DEFAULT_TCP_HOST ("localhost")

//------------------------------------------------------------------------------------------------------------------
/*
Ping : check if a destination can be reached
find an available port starting from a given port
*/
//------------------------------------------------------------------------------------------------------------------

bool               Ping(const char *ipAddress, std::string &FeedBack, DWORD iTimeout = 1000);
bool               IsTCPPortInUse(int port);
int                FindAvailableTCPPortNumber(int startPort);
std::vector<DWORD> ListTcpConnectionsForApp(const std::string &appName);

//--------------------------------------------------------------------------------------------------------------------------------------
/*
Resolve Ip strings like "127.0.0.1", "\\mycomputer", "www.test.com" to an IP address
*/
//--------------------------------------------------------------------------------------------------------------------------------------
bool ResolveIP4_Address(const std::string &HostName, std::string &IP_Number); // returns true on succes

typedef std::function<void()>               StateResponder; // function for state entry/run/exit
typedef std::function<void(SOCKET, size_t)> ConnectResponder;

#ifdef CTRACK
typedef std::function<void(CNode *&)> CommandResponder; // function for CNode responders
#endif

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

struct TReceiveBuffer
{
  public:
    TReceiveBuffer() { Reset(); }
    bool IsComplete() { return m_BytesLeft == 0; }
    bool IsInitialized() { return m_pBuffer != nullptr; };
    void Reset(char *pBuffer = nullptr, int SizeToRead = 0)
    {
        m_pBuffer   = pBuffer;
        m_BytesLeft = SizeToRead;
    }

  public:
    char *m_pBuffer{nullptr};
    int   m_BytesLeft{0};
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
    CSocket(SOCKET iSocket, E_COMMUNICATION_Mode, SOCKADDR_IN *ipSockAddress, int iUDPReceivePort, bool iUDPBroadcast, const std::string &iUDPSendToAddress,
            bool bDisableNagle = true);
    virtual ~CSocket();
    SOCKET GetSocket() { return m_Socket; };
    void   Throw(const std::string &ErrorMessage);
    void   ResetBuffers();

  public: // socket manipulations
    void SetUseHeader(bool bUseHeader = true) { m_bUseHeader = bUseHeader; };
    bool GetUseHeader() { return m_bUseHeader; };
    void DisableNagle(bool bDisableNagle = true);
    void SetNonBlocking(bool bNonBlocking = true);
    void SetReuseAddress(bool bEnableReuseAddress = true);
    void SetBroadcast(bool bEnableBroadcast = true);
    int  GetMaxUDPMessageSize();
    void SetBlockWrite(bool ibBlockWrite = true) { m_bBlockWrite = ibBlockWrite; };
    int  GetReadBufferSize();
    int  GetWriteBufferSize();
    void GetSocketBufferSizes(int &recvBufSize, int &sendBufSize);
    void SetSocketBufferSizes(int newRecvSize, int newSendSize);

  public:
    virtual bool DataAvailable(); // returns true if data is available for reading, throws FALSE if connection was reset or CExceptionSocket for socket error
    virtual int  TCPReceiveChunk(TReceiveBuffer &context, bool block);
    virtual bool ReadExtractTelegram(std::unique_ptr<CTCPGram> &ReturnTCPGram);
    virtual void TCPSendChunk(const char *, int);
    virtual bool WriteSendTelegram(
        std::unique_ptr<CTCPGram> &); // continues writing packets of the CTCPGram, when the complete TCPGram has been transmitted, then true
                                      // is returned, throws FALSE if connection was reset or CExceptionSocket for socket error
  protected:                          // socket and related
    SOCKET               m_Socket;
    E_COMMUNICATION_Mode m_CommunicationMode = TCP_SERVER;
    SOCKADDR_IN         *m_pSockAddress      = nullptr;
    struct sockaddr_in   m_UDPBroadCastAddr;
    bool                 m_bDisableNagle     = true; // if true : better latency , false : better throughput https://en.wikipedia.org/wiki/Nagle%27s_algorithm
    unsigned long        m_MaxUDPMessageSize = 0;    // only useful for UDP to generate an exception when the datagram is bigger than allowed

  protected: // read write buffers
    bool              m_bUseHeader = true;
    std::vector<char> m_Data; // data of the telegram
    // using header
    TMessageHeader    m_MessageHeader;           // header of the telegram
    TReceiveBuffer    m_ReceiveBuffer;           // buffer for receiving data
    bool              m_bHeaderReceived = false; // if true then the header has been received
    // using delimiter
    std::vector<char> m_Delimiter       = {'\n'}; // Delimiter for variable-size messages
    int               m_ChunkBufferSize = 1024;   // Read in chunks of 1024 bytes
    bool m_bBlockWrite = false; // if true then the socket will not write, this is used so that we can send a configuration first before sending anything else
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationInterface : class holding parameters, common parent for CCommunicationTCP and CCommunicationThread
This class also contains lists for incoming and outgoing telegrams and a list for newcomers. Newcomers are new client
sockets that connect to a server socket. The newcomer list allows to send a welcome message or take other actions when a
new client connects to a server.
All data members have thread safe acces (m_Lock)
*/
//------------------------------------------------------------------------------------------------------------------

class CCommunicationInterface : public CTrack::Subscriber
{
    friend class CCommunicationThread;

  public:
    void CopyFrom(CCommunicationInterface *ipFrom);

  public:
    CCommunicationInterface();
    E_COMMUNICATION_Mode GetCommunicationMode();
    void                 SetCommunicationMode(E_COMMUNICATION_Mode iMode);
    unsigned short       GetPort();
    unsigned short       GetPortUDP();
    bool                 GetUDPBroadcast();
    std::string          GetHostName();
    bool                 IsServer();
    bool                 ErrorOccurred();
    virtual std::string  GetError();
    virtual void         SetError(const std::string &iFileName, int iLineNumber, const std::string &iMessage);
    void                 SetOnConnectFunction(ConnectResponder iFunction) { m_OnConnectFunction = iFunction; };
    void                 SetOnDisconnectFunction(ConnectResponder iFunction) { m_OnDisconnectFunction = iFunction; };
    virtual bool         WaitConnection(DWORD timeoutMs) { return false; };

  public:
    void                                       SendMessage(CTrack::Message &);
    std::shared_ptr<CTrack::MessageResponder>  GetMessageResponder() const { return m_pMessageResponder; };
    std::shared_ptr<CTrack::MessageResponder> &GetMessageResponder() { return m_pMessageResponder; };
    [[nodiscard]] CTrack::Subscription         Subscribe(const std::string &id, CTrack::Handler);

  public: // CCommunicationParameters
    virtual size_t GetNumConnections() { return 0; };

  public: // sending data : pushes a telegram into the FIFO stack
    virtual bool GetSendPackage(
        std::unique_ptr<CTCPGram> &); // thrd: pops the oldest package from the start of the list, the returned CTCPGram needs to be destroyed afterwards
    virtual bool GetReceivePackage(std::unique_ptr<CTCPGram> &, const unsigned char Code = TCPGRAM_CODE_ALL);
    // CTCPGram needs to be destroyed afterwards
    virtual void PushSendPackage(std::unique_ptr<CTCPGram> &);    // Object: pushes a new send package to the end of the list
    virtual void PushReceivePackage(std::unique_ptr<CTCPGram> &); // thrd: pushes a new received package to the end of the list
    virtual void ClearBuffers();
    virtual void RemoveOldReceiveTelegrams(int iNumberToKeep); // removes old packages, keeps iNumberToKeep
  protected:                                                   // sockets and states
    E_COMMUNICATION_Mode m_CommunicationMode = TCP_SERVER;
    unsigned short       m_Port              = 50000; // default 50000
    unsigned short       m_PortUDP           = 50002; // default 50000
    std::string          m_HostName;                  // hostname, like "ctech-tower", "127.0.0.1" "localhost" "www.ctechmetrology.com"
    bool                 m_bUDPBroadCast = true;      // if false, m_IP4Address is used to send to
    bool                 m_bMakeBlocking = false;     // makes the socket blocking
    bool                 m_bDisableNagle = true;      // disables Nagle grouping of blocks to improve latency at the cost of througput
    float                m_TimeOut       = 0.5;       // time-out in seconds, only when blocking is activated
  protected:                                          // error handling
    bool              m_bErrorOccurred = false;       // set to true when an error occurred, further information in m_ErrorString
    std::string       m_ErrorString;
    std::string       m_ErrorSourceFile;
    int               m_ErrorSourceLine = 0;
    std::atomic<bool> m_bInitialized    = false;

  protected: // buffers
    std::list<std::unique_ptr<CTCPGram>> m_arSendBuffer;
    std::list<std::unique_ptr<CTCPGram>> m_arReceiveBuffer;
    std::recursive_mutex                 m_Mutex; // critical section to be used with CSingleLock to protect data
    ConnectResponder                     m_OnConnectFunction{};
    ConnectResponder                     m_OnDisconnectFunction{};

  protected:
    std::shared_ptr<CTrack::MessageResponder> m_pMessageResponder; // used to send messages to the UI
#ifdef CTRACK_UI
  public: // edit settings
    virtual void EditSettings(CPropertySheet *pPropertySheet = nullptr);
    virtual bool EditSettingsChanged();

  protected:
    std::unique_ptr<CDCommunicationPage> m_pPropertyPage;
#endif
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationObject manages 1 (UDP,Client) or multiple sockets(SERVER)
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
    virtual bool Open(E_COMMUNICATION_Mode iTcpMode = TCP_SERVER, int iPort = 40000, int iPortUDP = 0, const std::string &iIpAddress = ("127.0.0.1"));
    virtual void Close();

  public: // to be used in CNode derivations
    void XML_ReadWrite(TiXmlElement *&, bool Read /* XML_WRITE XML_READ */);
    void CopyFrom(CCommunicationObject *);

  public: // CCommunicationParameters
    size_t GetNumConnections() override;
    void   PushSendPackage(std::unique_ptr<CTCPGram> &) override;
    bool   WaitConnection(DWORD timeoutMs) override;

  public: // own overrideable functions
    virtual CSocket *SocketCreate(SOCKET iSocket, E_COMMUNICATION_Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPReceivePort, bool UDPBroadcast,
                                  const std::string &UDPSendPort); // CSocket* ipSocket,bool bAddToNewComerList = false);public: // get set ip4 related stuff
    void             SetCommunicationThread(std::shared_ptr<CCommunicationThread> &rCommunicationThread);
    void             SetThreadName(const std::string &iName) { m_ThreadName = iName; };

  protected:
    std::weak_ptr<CCommunicationThread> m_pCommunicationThread;
    std::mutex                          m_LockThreadRunning;
    bool                                m_bOpened = false;
    std::string                         m_ThreadName;

  public:
    static void AssignCommunicationThread(UINT, CCommunicationObject &);
    static void CloseCommunicationThread(UINT, CCommunicationObject &);
    static void CloseAllConnections();

  protected:
    static std::map<UINT /*port*/, std::shared_ptr<CCommunicationThread>> &getMapThreads()
    {
        static std::map<UINT /*port*/, std::shared_ptr<CCommunicationThread>> m_mapThreads;
        return m_mapThreads;
    };
    static std::mutex &getMapThreadsMutex()
    {
        static std::mutex m_MutexThreads;
        return m_MutexThreads;
    };
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationThread manages the communication thread. We keep one communication thread for one port, so if
multiple CCommunicationTCP objects want to communicate through the same port, they have to work with the same thread
*/
//------------------------------------------------------------------------------------------------------------------

class CCommunicationThread : public CCommunicationInterface
{
  public:
    CCommunicationThread();
    virtual ~CCommunicationThread();
    void   CommunicationObjectAdd(CCommunicationObject &);
    void   CommunicationObjectRemove(CCommunicationObject &);
    size_t CommunicationObjectGetNum();

  public: // CCommunicationParameters
    size_t GetNumConnections() override;
    void   PushReceivePackage(std::unique_ptr<CTCPGram> &) override;
    void   SetError(const std::string &iFileName, int iLineNumber, const std::string &iMessage) override;

  protected:
    void     SocketAdd(SOCKET iSocket, E_COMMUNICATION_Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPReceivePort, bool UDPBroadcast,
                       const std::string &UDPSendPort); // CSocket* ipSocket,bool bAddToNewComerList = false);
    CSocket *SocketFirst();                             // first in list, or NULL
    CSocket *SocketNext();                              // next, can only be called after SocketFirst
    CSocket *SocketDeleteCurrent();                     // deletes current socket and return pointer to next socket
    void     SocketDeleteAll();                         // deletes all sockets
  public:                                               // thread management
    void                     SetMessageResponderInstance(std::shared_ptr<CTrack::MessageResponder> responder);
    void                     ThreadFunction();
    void                     SetThreadName(const std::string &iName) { m_ThreadName = iName; }; // must be called before starting the thread
    void                     StartThread();
    void                     EndThread();
    void                     SetQuit(bool ibQuit = true);
    bool                     GetQuit();
    std::mutex              &GetConnectionMutex() { return m_connectionMutex; }
    std::condition_variable &GetConnectionCV() { return m_connectionCV; }

  protected: // thread members
    std::thread                                   m_Thread;
    std::atomic<bool>                             m_bQuit       = false;
    std::atomic<bool>                             m_bIntialized = false;
    std::list<std::unique_ptr<CSocket>>::iterator m_IterCurrentSocket;
    std::list<std::unique_ptr<CSocket>>           m_arSockets;
    std::set<CCommunicationObject *>              m_setCommunicationObject;
    std::string                                   m_ThreadName;
    std::condition_variable                       m_connectionCV;
    std::mutex                                    m_connectionMutex;
};
