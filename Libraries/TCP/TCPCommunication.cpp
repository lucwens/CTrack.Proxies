#include "TCPCommunication.h"
#include "../xml/TinyXML_AttributeValues.h"
#include "../Utility/os.h"
#include "../Utility/Print.h"
#include "../Utility/ErrorHandling.h"
#include "../Utility/StringUtilities.h"

#include <ws2tcpip.h>
#include <iostream>

#include <string>
#include <tlhelp32.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

#define RECEIVEBUFFERLENGTH 65535
#define MAX_DEBUG_TELEGRAMS 500
char ReceiveBuffer[RECEIVEBUFFERLENGTH];

std::atomic<bool> Interrupted{false};

void InterruptSet(bool bValue)
{
    Interrupted = bValue;
}
//------------------------------------------------------------------------------------------------------------------
/*
Supporting routines for the communication thread
*/
//------------------------------------------------------------------------------------------------------------------

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

#include <ws2tcpip.h>

// ...

bool Ping(const char *ipAddress, std::string &FeedBack, DWORD iTimeout)
{
    HANDLE         hIcmpFile;
    unsigned long  ipaddr      = INADDR_NONE;
    DWORD          dwRetVal    = 0;
    char           SendData[]  = "Data Buffer";
    LPVOID         ReplyBuffer = nullptr;
    DWORD          ReplySize   = 0;
    struct in_addr addr;

    if (InetPtonA(AF_INET, ipAddress, &addr) != 1)
    {
        FeedBack = "Invalid IP address";
        return false;
    }
    ipaddr    = addr.s_addr;

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE)
    {
        FeedBack = "Unable to open handle.";
        return false;
    }

    ReplySize   = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID *)malloc(ReplySize);
    if (ReplyBuffer == nullptr)
    {
        FeedBack = "Unable to allocate memory";
        return false;
    }

    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), nullptr, ReplyBuffer, ReplySize, iTimeout);
    if (dwRetVal != 0)
    {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr   ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ReplyAddr, ipStr, INET_ADDRSTRLEN);
        FeedBack = "Received " + std::to_string(pEchoReply->DataSize) + " bytes from " + std::string(ipStr) +
                   ": icmp_seq=" + std::to_string(pEchoReply->RoundTripTime) + " ms";

        free(ReplyBuffer);
        return true;
    }
    else
    {
        FeedBack = "Ping failed: " + std::to_string(GetLastError());
        free(ReplyBuffer);
        return false;
    }
}

bool ResolveIP4_Address(const std::string HostName, std::string &IP_Number)
{
    WSADATA wsaData;
    int     result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    addrinfo hints       = {};
    hints.ai_family      = AF_INET; // IPv4
    hints.ai_socktype    = SOCK_STREAM;
    hints.ai_protocol    = IPPROTO_TCP;

    addrinfo *resultList = nullptr;
    result               = getaddrinfo(HostName.c_str(), nullptr, &hints, &resultList);
    if (result != 0)
    {
        std::cerr << "getaddrinfo failed: " << result << std::endl;
        WSACleanup();
        return false;
    }

    for (addrinfo *ptr = resultList; ptr != nullptr; ptr = ptr->ai_next)
    {
        sockaddr_in *sockaddr_ipv4 = reinterpret_cast<sockaddr_in *>(ptr->ai_addr);
        char         ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipStr, sizeof(ipStr));
        IP_Number = ipStr;
        break; // Only take the first result
    }

    freeaddrinfo(resultList);
    WSACleanup();
    return true;
}

int FindAvailableTCPPort(const std::string &ipAddress, int startingPort)
{
    SOCKET      sock = INVALID_SOCKET;
    sockaddr_in addr{};
    int         availablePort = -1;

    sock                      = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return -1;
    }

    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ipAddress.c_str(), &addr.sin_addr);

    for (int port = startingPort; port <= 65535; ++port)
    {
        addr.sin_port = htons(port);

        if (::bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0)
        {
            // Found available port
            availablePort = port;
            break;
        }
    }

    // Clean up
    closesocket(sock);

    return availablePort;
}

std::string GetProcessNameByPID(DWORD pid)
{
    std::string processName = "Unknown";

    HANDLE hProcessSnap     = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return processName;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (pe32.th32ProcessID == pid)
            {
                std::wstring exeName = pe32.szExeFile;
                processName          = WstringToString(exeName);
                break;
            }
        }
        while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return processName;
}

std::vector<DWORD> ListTcpConnectionsForApp(const std::string &appName)
{
    DWORD                   dwSize   = 0;
    DWORD                   dwRetVal = 0;
    std::vector<DWORD>      returnVector;
    PMIB_TCPTABLE_OWNER_PID pTcpTable = nullptr;

    // Call GetExtendedTcpTable to get the size needed
    dwRetVal                          = GetExtendedTcpTable(nullptr, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    if (dwRetVal != ERROR_INSUFFICIENT_BUFFER)
    {
        std::cerr << "Failed to get buffer size for TCP table.\n";
        return returnVector;
    }

    pTcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);
    if (pTcpTable == nullptr)
    {
        std::cerr << "Memory allocation failed.\n";
        return returnVector;
    }

    // Get the TCP table
    dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    if (dwRetVal != NO_ERROR)
    {
        std::cerr << "GetExtendedTcpTable failed with error: " << dwRetVal << "\n";
        free(pTcpTable);
        return returnVector;
    }

    // Loop through the TCP table
    for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++)
    {
        DWORD       pid         = pTcpTable->table[i].dwOwningPid;
        std::string processName = GetProcessNameByPID(pid);

        if (processName == appName)
        {
            DWORD localPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
            returnVector.push_back(localPort);
            std::cout << "Process: " << processName << " (PID: " << pid << ") - Local Port: " << localPort << "\n";
        }
    }
    free(pTcpTable);
    return returnVector;
}

void DataAvailable(SOCKET connection, bool &bDataAvail, bool &bNetWorkError, long FAR &errval)
{
    timeval timeout = {0, 0};
    fd_set  readfds, exceptfds;

    bDataAvail    = false;
    bNetWorkError = false;

    FD_ZERO(&readfds);
    FD_SET(connection, &readfds);

    FD_ZERO(&exceptfds);
    FD_SET(connection, &exceptfds);

    // Check if there is data available
    int selectVal = select(0, &readfds, nullptr, nullptr, &timeout);
    if (SOCKET_ERROR != selectVal && 0 < selectVal)
        bDataAvail = true;

    // Check for errors
    selectVal = select(0, nullptr, nullptr, &exceptfds, &timeout);
    if (SOCKET_ERROR == selectVal || 0 < selectVal)
    {
        int FAR optlen = 4;
        getsockopt(connection, SOL_SOCKET, SO_ERROR, (char *)&errval, &optlen);
        bNetWorkError = true; // for errors check: ms-help://MS.MSDNQTR.2005OCT.1033/winsock/winsock/windows_sockets_error_codes_2.htm#winsock.wsaenotsock_2
    }
}

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationParameters
*/
//------------------------------------------------------------------------------------------------------------------

void CCommunicationInterface::CopyFrom(CCommunicationInterface *ipFrom)
{
    if (ipFrom)
    {
        m_CommunicationMode = ipFrom->m_CommunicationMode;
        m_Port              = ipFrom->m_Port;
        m_PortUDP           = ipFrom->m_PortUDP;
        m_HostName          = ipFrom->m_HostName;
        m_bUDPBroadCast     = ipFrom->m_bUDPBroadCast;
        m_bMakeBlocking     = ipFrom->m_bMakeBlocking;
        m_bEnableNagle      = ipFrom->m_bEnableNagle;
        m_TimeOut           = ipFrom->m_TimeOut;
    }
}

bool CCommunicationInterface::GetSendPackage(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    if (!m_arSendBuffer.empty())
    {
        ReturnTCPGram = std::move(m_arSendBuffer.front());
        m_arSendBuffer.pop_front();
        return true;
    }
    return false;
}

void CCommunicationInterface::RemoveOldReceiveTelegrams(int iNumberToKeep)
{
    while (m_arReceiveBuffer.size() > iNumberToKeep)
    {
        m_arReceiveBuffer.pop_front();
    }
}

bool CCommunicationInterface::GetReceivePackage(std::unique_ptr<CTCPGram> &ReturnTCPGram, unsigned char Code)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif
    if (!m_arReceiveBuffer.empty())
    {
        // iterate from front to back, check for valid code
        auto Iter = m_arReceiveBuffer.begin();
        while (Iter != m_arReceiveBuffer.end())
        {
            bool bValidCode = true;
            if (Code != TCPGRAM_CODE_DONT_USE)
            {
                if ((*Iter)->GetCode() != Code)
                    bValidCode = false;
            }
            if (bValidCode)
            {
                ReturnTCPGram = std::move(*Iter);
                m_arReceiveBuffer.erase(Iter);
                return true;
            }
            Iter++;
        }
    }
    return false;
}

bool CCommunicationInterface::GetLastReceivePackage(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif
    if (!m_arReceiveBuffer.empty())
    {
        ReturnTCPGram = std::move(m_arReceiveBuffer.front());
        m_arReceiveBuffer.pop_front();
        return true;
    }
    return false;
}

void CCommunicationInterface::PushSendPackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_arSendBuffer.emplace_back(std::move(rTCPGram));
}

void CCommunicationInterface::PushReceivePackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif
    m_arReceiveBuffer.emplace_back(std::move(rTCPGram));
}

void CCommunicationInterface::ClearBuffers()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_arSendBuffer.clear();
    m_arReceiveBuffer.clear();
}

bool CCommunicationInterface::IsServer()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return (m_CommunicationMode == TCP_SERVER);
}

bool CCommunicationInterface::ErrorOccurred()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_bErrorOccurred;
}

std::string CCommunicationInterface::GetError()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_ErrorString;
}

void CCommunicationInterface::SetError(const std::string iFileName, int iLineNumber, const std::string iMessage)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_bErrorOccurred  = true;
    m_ErrorString     = iMessage;
    m_ErrorSourceFile = iFileName;
    m_ErrorSourceLine = iLineNumber;
}

unsigned short CCommunicationInterface::GetPort()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_Port;
}

unsigned short CCommunicationInterface::GetPortUDP()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_PortUDP;
}

bool CCommunicationInterface::GetUDPBroadcast()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_bUDPBroadCast;
}

std::string CCommunicationInterface::GetHostName()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_HostName;
}

E_COMMUNICATION_Mode CCommunicationInterface::GetCommunicationMode()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_CommunicationMode;
}

void CCommunicationInterface::SetCommunicationMode(E_COMMUNICATION_Mode iMode)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_CommunicationMode = iMode;
}

void CCommunicationInterface::AddNewComer(CSocket *iSocket)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_arNewComers.push_back(iSocket);
}

CSocket *CCommunicationInterface::GetNewComer()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    CSocket                              *ReturnSocket = nullptr;
    auto                                  iter         = m_arNewComers.begin();
    if (iter != m_arNewComers.end())
    {
        ReturnSocket = *iter;
        m_arNewComers.erase(iter);
        return ReturnSocket;
    }
    return ReturnSocket;
}

//------------------------------------------------------------------------------------------------------------------
/*
Communication interface class
*/
//------------------------------------------------------------------------------------------------------------------

#define ATTRIB_TCP_MODE        ("tcp_mode")
#define ATTRIB_PORT            ("port")
#define ATTRIB_PORT_UDP        ("port_udp")
#define ATTRIB_SEND_BLOCKING   ("blocking")
#define ATTRIB_SEND_NAGLE      ("nagle")
#define ATTRIB_SEND_TIME_OUT   ("time_out")
#define ATTRIB_UDP_BROADCAST   ("udp_broadcast")
#define ATTRIB_UDP_DESTINATION ("udp_destination")

CCommunicationObject::CCommunicationObject()
{
    m_bErrorOccurred    = false;
    m_CommunicationMode = TCP_SERVER;
    m_Port              = DEFAULT_TCP_PORT;
    m_HostName          = DEFAULT_TCP_HOST;
}

CCommunicationObject::~CCommunicationObject()
{
    Close();
    ClearBuffers();
}

void CCommunicationObject::XML_ReadWrite(TiXmlElement *&pXML, bool Read)
{
    int Mode = static_cast<int>(m_CommunicationMode);
    GetSetAttribute(pXML, ATTRIB_TCP_MODE, Mode, Read);
    m_CommunicationMode = static_cast<E_COMMUNICATION_Mode>(Mode);

    GetSetAttribute(pXML, ATTRIB_PORT, m_Port, Read);
    GetSetAttribute(pXML, ATTRIB_PORT_UDP, m_PortUDP, Read);
    GetSetAttribute(pXML, ATTRIB_SEND_BLOCKING, m_bMakeBlocking, Read);
    GetSetAttribute(pXML, ATTRIB_SEND_NAGLE, m_bEnableNagle, Read);
    GetSetAttribute(pXML, ATTRIB_SEND_TIME_OUT, m_TimeOut, Read);
    GetSetAttribute(pXML, ATTRIB_UDP_BROADCAST, m_bUDPBroadCast, Read);
    GetSetAttribute(pXML, ATTRIB_UDP_DESTINATION, m_HostName, Read);
}

void CCommunicationObject::CopyFrom(CCommunicationObject *ipNode)
{
    CCommunicationInterface *pCommunicationParameters = dynamic_cast<CCommunicationInterface *>(ipNode);
    if (pCommunicationParameters)
        CCommunicationInterface::CopyFrom(pCommunicationParameters);
}

void CCommunicationObject::Open(E_COMMUNICATION_Mode iTcpMode, int iPort, int iPortUDP, const std::string iIpAddress)
{
    // start the communication thread
    m_CommunicationMode = iTcpMode;
    m_Port              = iPort;
    m_PortUDP           = iPortUDP;
    m_HostName          = iIpAddress;

    Close();
    if (!m_pCommunicationThread)
    {
        // when a communication object wants to open the communication channels, CCommunicationManager checks if there is already a thread running on the same
        // port, if yes then the communication object is added to the list of communication objects belonging to that port, if no, then we create a new thread

        m_pCommunicationThread = std::make_shared<CCommunicationThread>();
        m_pCommunicationThread->CommunicationObjectAdd(*this);
        m_pCommunicationThread->StartThread();
    };
}

void CCommunicationObject::Close()
{
    // shut-down the communication thread
    if (m_pCommunicationThread)
    {
        m_pCommunicationThread->EndThread();
        m_pCommunicationThread.reset();
    };
}

void CCommunicationObject::CheckConnections()
{
    size_t NewNumConnections = GetNumConnections();
    if (NewNumConnections != m_TCPNumConnections)
    {
        CSocket *pNewSocket = GetNewComer(); // newcomers are only produced on server sockets
        while (pNewSocket != nullptr)
        {
            // standard response of server to newly connecting client : send the configuration
            // we only send the configuration when we are running
            pNewSocket = GetNewComer();
        }
        m_TCPNumConnections = NewNumConnections;
        // optional responder
        if (m_TCPConnectChange)
            m_TCPConnectChange(m_TCPNumConnections);
    }
}

void CCommunicationObject::TCPReceiveRespond()
{
    std::unique_ptr<CTCPGram> Telegram;
    bool                      bAvailable = GetReceivePackage(Telegram);
    while (bAvailable)
    {
        unsigned short Code = Telegram->GetCode();
        switch (Code)
        {
            case TCPGRAM_CODE_COMMAND:
            {
                std::unique_ptr<TiXmlElement> XML = Telegram->GetXML();
                // 			CNode* pNode = GetNodeFactory()->CreateNodeFromXML(XML.get(), true);
                // 			if (pNode)
                // 			{
                // 				CommandExecute(pNode);
                // 			}
                // 			else
                std::cout << "Failed converting the node from XML" << std::endl;
            };
            break;
            case TCPGRAM_CODE_STATUS: // state changing is only allowed by the server, so normally we do not receive states here
            {
                std::unique_ptr<TiXmlElement> XML = Telegram->GetXML();
                assert(XML.get() != NULL);
                // 			std::string StateName = StripName(XML->Value());
                // 			StateUpdate(StateName, XML);
                // 			if (m_bTCPServer) // reflect our state change immediate if we are server, probably a client is resetting an error state
                // 				TCPSendStatus(true);
            };
            break;
        }
        // next message
        bAvailable = GetReceivePackage(Telegram);
    }
}

void CCommunicationObject::Run()
{
    // this is all we do
    CheckConnections(); // check if new clients are connecting
    TCPReceiveRespond();
    IdleRun();
}

void CCommunicationObject::IdleRun()
{
    // 		for (auto iter : m_IdleTaskmap)
    // 		{
    // 			iter.second->Execute();
    // 		}
    //
    // 		// the API Innovo task wants to kill itself if it sees that no more messages pass through the message pump
    // 		// if we kill it in the loop above, then the iteration runs broke
    // 		for (auto& iter : m_IdleTaskToRemove)
    // 		{
    // 			auto iter_map = m_IdleTaskmap.find(iter);
    // 			if (iter_map != m_IdleTaskmap.end())
    // 			{
    // 				m_IdleTaskmap.erase(iter_map);
    // 				delete iter;
    // 			}
    // 		}
    // 		m_IdleTaskToRemove.clear();
}

CSocket *CCommunicationObject::SocketCreate(SOCKET iSocket, E_COMMUNICATION_Mode Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPBroadCastPort,
                                            bool UDPBroadcast, const std::string UDPSendAddress)
{
    return new CSocket(iSocket, Mode, ipSockAddress, UDPBroadCastPort, UDPBroadcast, UDPSendAddress);
}

size_t CCommunicationObject::GetNumConnections()
{
    if (m_pCommunicationThread)
        return 1;
    else
        return 0;
}

void CCommunicationObject::PushSendPackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    if (m_pCommunicationThread)
        m_pCommunicationThread->PushSendPackage(rTCPGram);
}

//------------------------------------------------------------------------------------------------------------------
/*
CSocket class
*/
//------------------------------------------------------------------------------------------------------------------

CSocket::CSocket(SOCKET iSocket, E_COMMUNICATION_Mode iCommunicationMode, SOCKADDR_IN *ipSockAddress, int UDBroadCastPort, bool iUDPBroadcast,
                 const std::string iUDPSendToAddress)
{
    m_Socket              = iSocket;
    m_CommunicationMode   = iCommunicationMode;
    m_pSockAddress        = ipSockAddress;
    m_CurrentTelegramSize = 0;

    m_TotalBytesWritten   = 0;
    m_MaxUDPMessageSize   = 0;

    ZeroMemory(&m_UDPBroadCastAddr, sizeof(m_UDPBroadCastAddr));
    if (iUDPBroadcast)
        m_UDPBroadCastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    else
        inet_pton(AF_INET, iUDPSendToAddress.c_str(), &(m_UDPBroadCastAddr.sin_addr.s_addr));

    m_UDPBroadCastAddr.sin_family = AF_INET;                // Address format is IPv4
    m_UDPBroadCastAddr.sin_port   = htons(UDBroadCastPort); // Convert from little to big endian

    switch (m_CommunicationMode)
    {
        case TCP_SERVER:
        case TCP_CLIENT:
        {
            SetNagle();
            SetNonBlocking();
            SetReuseAddress();
        };
        break;
        case UDP:
        {
            SetNonBlocking();
            // 			SetReuseAddress();
            SetBroadcast();
            m_MaxUDPMessageSize = GetMaxUDPMessageSize();
        };
        break;
    }
}

CSocket::~CSocket()
{
    shutdown(m_Socket, SD_BOTH);
    closesocket(m_Socket);
}

void CSocket::SetNagle(bool bDisableNagle)
{
    int optval = (bDisableNagle ? 1 : 0);
    if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&optval, sizeof(optval)) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(("Failed to set the Nagle option"), LastError);
    }
}

void CSocket::SetNonBlocking(bool bNonBlocking)
{
    unsigned long argList[1];
    argList[0] = (bNonBlocking ? 0 : 1);
    if (ioctlsocket(m_Socket, FIONBIO, argList) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(("Failed to set non-blocking option"), LastError);
    }
}

void CSocket::SetReuseAddress(bool bEnableReuseAddress)
{
    int on = (bEnableReuseAddress ? 1 : 0);
    if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(("Failed to set the reuse address option"), LastError);
    }
}

void CSocket::SetBroadcast(bool bEnableBroadcast)
{
    int optval = (bEnableBroadcast ? 1 : 0);
    if (setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(("Failed to set the broadcast option"), LastError);
    }
}

int CSocket::GetMaxUDPMessageSize()
{
    int maxSize = 0;
    int SizeOf  = sizeof(int);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&maxSize, &SizeOf) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(("Failed to set the reuse address option"), LastError);
    }
    return maxSize;
}

bool CSocket::DataAvailable()
{
    long    errval;
    timeval timeout = {0, 0};
    fd_set  readfds, exceptfds;

    FD_ZERO(&readfds);
    FD_SET(m_Socket, &readfds);

    FD_ZERO(&exceptfds);
    FD_SET(m_Socket, &exceptfds);

    // Check if there is data available
    int selectVal = select(0, &readfds, nullptr, nullptr, &timeout);
    if (SOCKET_ERROR != selectVal && 0 < selectVal)
        return true;

    // Check if there was an error
    selectVal = select(0, nullptr, nullptr, &exceptfds, &timeout);
    if (SOCKET_ERROR == selectVal || 0 < selectVal)
    {
        int FAR optlen = 4;
        getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, (char *)&errval, &optlen);
        if (errval == WSAECONNRESET || errval == WSAENOTCONN || errval == WSAECONNABORTED)
            throw false; // connection was closed
        else
            THROW_SOCKET_ERROR(("An error occurred on the TCP network"), errval);
    }
    return false;
}

void CSocket::ReadFeedBuffer()
{
    // check for data availability
    bool     bDataAvail(false), bNetWorkError(false);
    long FAR errval(0);
    if (!DataAvailable())
    {
        Sleep(1);
        return;
    }

    // read into our buffer and check connection
    int rNumReceived = recv(m_Socket, ReceiveBuffer, RECEIVEBUFFERLENGTH, 0);
    if ((rNumReceived == SOCKET_ERROR) || (rNumReceived == 0))
    {
        int LastError = WSAGetLastError();
        if (LastError == WSAECONNRESET || LastError == WSAENOTCONN || LastError == WSAECONNABORTED || (rNumReceived == 0))
            throw false; // connection was closed
        else
            THROW_SOCKET_ERROR(("An error occurred trying to read data from the network"), LastError);
    }
    else
    {
        std::vector<std::uint8_t> appendBuffer(ReceiveBuffer, ReceiveBuffer + rNumReceived);
        AppendBack(m_AccumulationBuffer, appendBuffer);
    }
}

bool CSocket::ReadExtractTelegram(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    if (m_AccumulationBuffer.empty())
        return false; // nothing in the buffer

    if (m_CurrentTelegramSize == 0 && m_AccumulationBuffer.size() >= TCPGRAM_HEADER_SIZE) // start of a new telegram
        m_CurrentTelegramSize = GetSize(m_AccumulationBuffer);

    if (m_AccumulationBuffer.size() >= m_CurrentTelegramSize) // new telegram ready
    {
        std::vector<std::uint8_t> telegramData = RemoveFront(m_AccumulationBuffer, m_CurrentTelegramSize);
        ReturnTCPGram.reset(new CTCPGram(telegramData));
        m_CurrentTelegramSize = 0;
        return true;
    }
    return false;
}

void CSocket::WriteSendReset()
{
    m_TotalBytesWritten = 0;
}

bool CSocket::WriteSend(std::unique_ptr<CTCPGram> &rTCPGram)
{
    if (m_bBlockWrite)
        return true;
    switch (m_CommunicationMode)
    {
        case TCP_CLIENT:
        case TCP_SERVER:
        {
            // try to send as much as possible
            int NumBytesWritten = send(m_Socket, reinterpret_cast<const char *>(rTCPGram->m_Data.data()), static_cast<int>(rTCPGram->m_Data.size()), 0);
            if (NumBytesWritten != SOCKET_ERROR)
            {
                m_TotalBytesWritten += NumBytesWritten;
                bool bFinished = (m_TotalBytesWritten == rTCPGram->m_Data.size());
                if (bFinished)
                    m_TotalBytesWritten = 0;

                return bFinished;
            }
            else // if the connection was closed, then remove the socket from the array
            {
                int LastError = WSAGetLastError();
                if (LastError == WSAECONNRESET || LastError == WSAENOTCONN || LastError == WSAECONNABORTED)
                    throw false; // connection was closed
                else
                    THROW_SOCKET_ERROR(("An error occurred trying to send data over the TCP network"), LastError);
            }
        };
        break;
        case UDP:
        {
            if (rTCPGram->m_Data.size() > m_MaxUDPMessageSize)
            {
                std::string ErrorMessage =
                    fmt::format("Error sending data over UDP : the package size {} is bigger than allowed {}", rTCPGram->m_Data.size(), m_MaxUDPMessageSize);
                THROW_ERROR(ErrorMessage);
            }
            int rVal = ::sendto(m_Socket, reinterpret_cast<const char *>(rTCPGram->m_Data.data()), static_cast<int>(rTCPGram->m_Data.size()), 0,
                                (SOCKADDR *)&m_UDPBroadCastAddr, sizeof(m_UDPBroadCastAddr));
            if (rVal == SOCKET_ERROR)
            {
                int LastError = WSAGetLastError();
                THROW_SOCKET_ERROR(("An error occurred trying to send data over the UDP network"), LastError);
            }
            else
                return (rVal == rTCPGram->m_Data.size());
        };
        break;
    }
    return false;
}

void CSocket::AppendBack(std::vector<std::uint8_t> &mainBuffer, const std::vector<std::uint8_t> &appendBuffer)
{
    if (appendBuffer.empty())
        return;

    mainBuffer.insert(mainBuffer.end(), appendBuffer.begin(), appendBuffer.end());
}

std::vector<std::uint8_t> CSocket::RemoveFront(std::vector<std::uint8_t> &mainBuffer, unsigned long sizeToRemove)
{
    std::vector<std::uint8_t> returnBuffer;

    if (sizeToRemove > mainBuffer.size() || sizeToRemove == 0)
        return returnBuffer;

    returnBuffer.assign(mainBuffer.begin(), mainBuffer.begin() + sizeToRemove);
    mainBuffer.erase(mainBuffer.begin(), mainBuffer.begin() + sizeToRemove);

    return returnBuffer;
}

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationThread
*/
//------------------------------------------------------------------------------------------------------------------

CCommunicationThread::CCommunicationThread()
{
    m_bErrorOccurred    = false;
    m_CommunicationMode = TCP_SERVER;
    m_Port              = DEFAULT_TCP_PORT;
    m_HostName          = DEFAULT_TCP_HOST;
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_IterCurrentSocket = m_arSockets.begin();
}

CCommunicationThread::~CCommunicationThread()
{
}

void CCommunicationThread::SetQuit(bool ibQuit)
{
    m_bQuit = ibQuit;
}

bool CCommunicationThread::GetQuit()
{
    return m_bQuit;
}

void CCommunicationThread::CommunicationObjectAdd(CCommunicationObject &rCommunicationObject)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_setCommunicationObject.insert(&rCommunicationObject);
    m_Port    = rCommunicationObject.GetPort();
    m_PortUDP = rCommunicationObject.GetPortUDP();
}

void CCommunicationThread::CommunicationObjectRemove(CCommunicationObject &rCommunicationObject)
{
    {
        std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
        m_setCommunicationObject.erase(&rCommunicationObject);
    }
    if (CommunicationObjectGetNum() == 0)
    {
        EndThread();
    }
}

size_t CCommunicationThread::CommunicationObjectGetNum()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_setCommunicationObject.size();
}

void CCommunicationThread::SocketAdd(SOCKET iSocket, E_COMMUNICATION_Mode Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPBroadCastPort,
                                     bool bAddToNewComerList, bool UDPBroadcast, const std::string UDPSendPort)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    // a communication thread can have multiple ccommunicationObjects, which may have different SocketCreate methods, here we just take the first
    auto                                  iter = m_setCommunicationObject.begin();
    if (iter != m_setCommunicationObject.end())
    {
        CSocket *pNewSocket = (*iter)->SocketCreate(iSocket, Mode, ipSockAddress, UDPBroadCastPort, UDPBroadcast, UDPSendPort);
        m_arSockets.push_back(pNewSocket);
        if (bAddToNewComerList)
            AddNewComer(pNewSocket);
    }
    else
        assert(false); // existing port
}

CSocket *CCommunicationThread::SocketFirst()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_IterCurrentSocket = m_arSockets.begin();
    if (m_IterCurrentSocket != m_arSockets.end())
        return (*m_IterCurrentSocket);
    else
        return nullptr;
}

CSocket *CCommunicationThread::SocketNext()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    m_IterCurrentSocket++;
    if (m_IterCurrentSocket != m_arSockets.end())
        return (*m_IterCurrentSocket);
    else
        return nullptr;
}

CSocket *CCommunicationThread::SocketDeleteCurrent()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    if (m_IterCurrentSocket != m_arSockets.end())
    {
        delete *m_IterCurrentSocket;
        m_IterCurrentSocket = m_arSockets.erase(m_IterCurrentSocket);
        if (m_IterCurrentSocket != m_arSockets.end())
            return (*m_IterCurrentSocket);
        else
            return nullptr;
    }
    return nullptr;
}

void CCommunicationThread::SocketDeleteAll()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    for (auto &arSocket : m_arSockets)
        delete arSocket;
    m_arSockets.clear();
}

void CCommunicationThread::SocketResetSend()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    for (auto &arSocket : m_arSockets)
        arSocket->WriteSendReset();
}

void CCommunicationThread::AddNewComer(CSocket *iSocket)
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    //
    // do not store locally, copy all to CCommunicationObjects
    for (auto iter : m_setCommunicationObject)
        iter->AddNewComer(iSocket);
}

size_t CCommunicationThread::GetNumConnections()
{
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    return m_arSockets.size();
}

void CCommunicationThread::PushReceivePackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    // copy incoming telegrams to all CCommunicationObjects
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    for (auto iter : m_setCommunicationObject)
    {
        std::unique_ptr<CTCPGram> CopyTCPGram = std::make_unique<CTCPGram>();
        CopyTCPGram->CopyFrom(rTCPGram);
        iter->PushReceivePackage(CopyTCPGram);
    }
}

void CCommunicationThread::SetError(const std::string iFileName, int iLineNumber, const std::string iMessage)
{
    // copy error to all CCommunicationObjects
    std::lock_guard<std::recursive_mutex> Lock(m_Mutex);
    PrintError("error occurred in %s at %d : %s ", iFileName.c_str(), iLineNumber, iMessage.c_str());
    for (auto iter : m_setCommunicationObject)
        iter->SetError(iFileName, iLineNumber, iMessage);
}

void CCommunicationThread::StartThread()
{
    if (!m_Thread.joinable())
    {
        m_Thread = std::thread(&CCommunicationThread::ThreadFunction, this);
    }
}

void CCommunicationThread::EndThread()
{
    SetQuit(true);
    if (m_Thread.joinable())
        m_Thread.join();
}

//------------------------------------------------------------------------------------------------------------------
/*
This is the main communication thread
*/
//------------------------------------------------------------------------------------------------------------------
void CCommunicationThread::ThreadFunction()
{
    SOCKET               MainSocket = NULL; // for server mode : listen socket / client mode : client socket / udp : communication socket
    SOCKADDR_IN          sincontrol;
    E_COMMUNICATION_Mode CommunicationMode = GetCommunicationMode();
    WORD                 sockVersion;
    WSADATA              wsaData;

    try
    {
        SetQuit(false);
        std::string ThreadName(("CCommunicationThread"));
        SetThreadName(ThreadName);

        //--------------------------------------------------------------------------------------------------------
        // Initialize socket library
        //--------------------------------------------------------------------------------------------------------
        sockVersion = MAKEWORD(2, 2);
        // start dll
        if (WSAStartup(sockVersion, &wsaData) != 0)
            THROW_ERROR(("Failed to initialize the socket library"));

        //--------------------------------------------------------------------------------------------------------
        // Resolve IP4 address (no need for IP6)
        //--------------------------------------------------------------------------------------------------------
        std::string    IP4;
        std::string    HostName      = GetHostName();
        unsigned short PortNumber    = GetPort();
        unsigned short PortNumberUDP = GetPortUDP();
        bool           bUDPBroadcast = GetUDPBroadcast();

        if (!ResolveIP4_Address(HostName, IP4))
        {
            std::string ErrorMessage = fmt::format("The host {} could not be resolved", HostName);
            THROW_ERROR(ErrorMessage);
        }

        //--------------------------------------------------------------------------------------------------------
        // create the socket
        //--------------------------------------------------------------------------------------------------------
        switch (CommunicationMode)
        {
            case TCP_CLIENT:
            {

                ZeroMemory(&sincontrol, sizeof(sincontrol));
                sincontrol.sin_family = PF_INET;
                sincontrol.sin_port   = htons(PortNumber);
                inet_pton(AF_INET, IP4.c_str(), &(sincontrol.sin_addr.s_addr));
                MainSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (MainSocket == INVALID_SOCKET)
                {
                    int         LastError    = WSAGetLastError();
                    std::string ErrorMessage = fmt::format("The creation of the socket (host : {} port:{}) failed.", HostName.c_str(), PortNumber);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                };
                std::cout << "TCP client on port " << PortNumber << std::endl;
            };
            break;
            case TCP_SERVER:
            {
                ZeroMemory(&sincontrol, sizeof(sincontrol));
                sincontrol.sin_family      = PF_INET;
                sincontrol.sin_port        = htons(PortNumber);
                sincontrol.sin_addr.s_addr = INADDR_ANY;
                MainSocket                 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (MainSocket == INVALID_SOCKET)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The creation of the socket (host : {} port:{}) failed. Lasterror was {}.", HostName.c_str(), PortNumber, LastError);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // make it NON BLOCKING
                unsigned long argList[1];
                argList[0] = 1;
                int rVal   = ioctlsocket(MainSocket, FIONBIO, argList);
                if (rVal == SOCKET_ERROR)
                {
                    int         LastError    = WSAGetLastError();
                    std::string ErrorMessage = fmt::format("The creation of the socket (host : {} port:{}) failed.", HostName.c_str(), PortNumber);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The binding of the socket (host : {} port:%d) failed.Lasterror was {}.\nPossibly the port is already in use.",
                                    HostName.c_str(), PortNumber, LastError);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
                //
                // start listening
                if (::listen(MainSocket, 1 /*backlog amount permitted*/) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The listen command on the socket (host : {} port:{}) failed.Lasterror was {}.\nPossibly the port is already in use.",
                                    HostName.c_str(), PortNumber, LastError);
                    std::cout << ErrorMessage << std::endl;
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
            };
            break;
            case UDP: // udp is connectionless, so it cannot be disconnected, so no need to check this in the big loop
            {
                ZeroMemory(&sincontrol, sizeof(sincontrol));
                sincontrol.sin_family      = AF_INET;
                sincontrol.sin_port        = htons(PortNumberUDP); // since this will be used to listen for commands coming form clients
                sincontrol.sin_addr.s_addr = htonl(INADDR_ANY);
                MainSocket                 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (MainSocket == INVALID_SOCKET)
                {
                    int         LastError    = WSAGetLastError();
                    std::string ErrorMessage = fmt::format("The creation of the socket (host : {} port:{}) failed.", HostName.c_str(), PortNumberUDP);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The binding of the socket (host : {} port:{}) failed.Lasterror was {}.\nPossibly the port is already in use.",
                                    HostName.c_str(), PortNumberUDP, LastError);
                    std::cout << ErrorMessage << std::endl;
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
                SocketAdd(MainSocket, UDP, &sincontrol, PortNumber, false, bUDPBroadcast, HostName);

                std::cout << "UDP available on port " << PortNumber << std::endl;
            };
            break;
        }

        //--------------------------------------------------------------------------------------------------------
        // BIG LOOP
        //--------------------------------------------------------------------------------------------------------
        /*
        BIG Loop
        - Check connection state
        - Server:
        - Check data-available on listen socket, if true
        -
        - Send data
        - Receive data
        */
        while (true)
        {
            if (GetQuit())
                break;
            //--------------------------------------------------------------------------------------------------------
            // Connection checking for TCP server and client
            //--------------------------------------------------------------------------------------------------------
            switch (CommunicationMode)
            {
                case TCP_SERVER:
                {
                    // check if a data request is there on the listensocket
                    bool     bDataAvail(false), bNetWorkError(false);
                    long FAR errval(0);
                    DataAvailable(MainSocket, bDataAvail, bNetWorkError,
                                  errval); // a client trying to connect on our server will send data to the listen socket
                    if (bDataAvail)
                    {
                        SOCKET ClientSocket = accept(MainSocket, nullptr, nullptr);
                        if (ClientSocket != SOCKET_ERROR)
                        {
                            // callback for freshly connected sockets : send the configuration if the engine is running
                            PrintInfo("Client accepted at port " + std::to_string(PortNumber));
                            SocketAdd(ClientSocket, TCP_SERVER, &sincontrol, 0, true, false, (""));
                        }
                    }
                };
                break;
                case TCP_CLIENT:
                {
                    if (GetNumConnections() == 0)
                    {
                        if (MainSocket == INVALID_SOCKET)
                        {
                            MainSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                            if (MainSocket == INVALID_SOCKET)
                            {
                                int         LastError    = WSAGetLastError();
                                std::string ErrorMessage = fmt::format("The creation of the socket (host : {} port:{}) failed.", HostName.c_str(), PortNumber);
                                THROW_SOCKET_ERROR(ErrorMessage, LastError);
                            }
                            ZeroMemory(&sincontrol, sizeof(sincontrol));
                            sincontrol.sin_family = PF_INET;
                            sincontrol.sin_port   = htons(PortNumber);
                            inet_pton(AF_INET, IP4.c_str(), &(sincontrol.sin_addr.s_addr));
                        }
                        if (connect(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) != SOCKET_ERROR)
                        {
                            PrintInfo("TCP client connected to ", HostName, " on port ", PortNumber);
                            SocketAdd(MainSocket, TCP_CLIENT, &sincontrol, 0, false, false, (""));
                        }
                        else
                        {
                            int SocketError = WSAGetLastError();
                            if ((SocketError != WSAECONNREFUSED) && (SocketError != WSAEWOULDBLOCK) && (SocketError != WSAEALREADY))
                                THROW_SOCKET_ERROR(("An error occurred trying to connect to the server"), SocketError);
                            else
                                ::Sleep(1000); // don't check every microsecond
                        }
                    }
                };
                break;
            }

            //--------------------------------------------------------------------------------------------------------
            // Sending data : loop over send data buffer, loop over connections until complete telegram is send, then get
            // the next telegram, until all telegrams have been send
            // if sending is not possible because of closed connection, remove the socket from arConnectionSocket
            //--------------------------------------------------------------------------------------------------------
            std::unique_ptr<CTCPGram> TCPGram;
            bool                      bAvailable = GetSendPackage(TCPGram);
            while (bAvailable)
            {
                bool bAllSocketsCompleted = true;
                if (TCPGram)
                {
                    SOCKET   Destination    = TCPGram->GetDestination();
                    CSocket *pCurrentSocket = SocketFirst();
                    while (pCurrentSocket != nullptr)
                    {
                        try
                        {
                            if (Destination == ALL_DESTINATIONS || Destination == pCurrentSocket->GetSocket())
                                if (!pCurrentSocket->WriteSend(TCPGram)) // returns false if sending of the telegram was not completed yet
                                    bAllSocketsCompleted = false;
                            pCurrentSocket = SocketNext();
                        }
                        catch (bool &) // disconnected, other exceptions are handled by outer routines
                        {
                            pCurrentSocket = SocketDeleteCurrent();
                            if (CommunicationMode == TCP_CLIENT)
                            {
                                PrintWarning("TCP client disconnected from ", HostName, " on port ", PortNumber);
                                MainSocket = INVALID_SOCKET;
                            }
                        }
                    }
                }
                if (bAllSocketsCompleted) // get the next package
                {
                    bAvailable = GetSendPackage(TCPGram);
                    SocketResetSend();
                }
            }

            //--------------------------------------------------------------------------------------------------------
            // Receiving data
            //--------------------------------------------------------------------------------------------------------
            CSocket *pCurrentSocket = SocketFirst();
            while (pCurrentSocket != nullptr)
            {
                try
                {
                    pCurrentSocket->ReadFeedBuffer();
                    std::unique_ptr<CTCPGram> TCPGram;
                    bool                      bAvailable = pCurrentSocket->ReadExtractTelegram(TCPGram);
                    while (bAvailable)
                    {
                        PushReceivePackage(TCPGram);
                        bAvailable = pCurrentSocket->ReadExtractTelegram(TCPGram);
                    };
                    pCurrentSocket = SocketNext();
                }
                catch (bool &) // disconnected, other exceptions are handled by outer routines
                {
                    pCurrentSocket = SocketDeleteCurrent();
                    PrintWarning("TCP client disconnected from {} on port {}", HostName, PortNumber);
                    if (CommunicationMode == TCP_CLIENT)
                    {
                        MainSocket = INVALID_SOCKET;
                    }
                }
            }
        }
#ifdef _DEBUG
        PrintInfo("Closing thread for port : ", PortNumber);
#endif
    }
    catch (const std::exception &e)
    {
        SetError(__FILE__, __LINE__, e.what());
    }
    catch (...)
    {
        SetError(__FILE__, __LINE__, "An unknown error occurred");
    }

    //
    // close our sockets
    SocketDeleteAll();

    if (CommunicationMode == TCP_SERVER)
    {
        shutdown(MainSocket, SD_BOTH);
        closesocket(MainSocket); // close the listen socket
    }
    //
    // clean up what ever is left in the buffers

    //--------------------------------------------------------------------------------------------------------
    //
    // Close socket library
    //
    //--------------------------------------------------------------------------------------------------------
    //
    // close the library
    WSACleanup();
}
