#ifdef CTRACK

#include "stdafx.h"

#include "../Proxies/Libraries/TCP/TCPCommunication.h"
#include "ProcessRoutines.h"
#include "Print.h"
#include "Interrupt.h"
#include "EngineMessages.h"

#include <Icmpapi.h>
#include <Ws2tcpip.h>
#include <tlhelp32.h>

#else

#include "TCPCommunication.h"
#include "../xml/TinyXML_AttributeValues.h"
#include "../Utility/os.h"
#include "../Utility/Print.h"
#include "../Utility/errorException.h"
#include "../Utility/StringUtilities.h"
#include "../Utility/logging.h"

#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

std::atomic<bool> Interrupted{false};

void InterruptSet(bool bValue)
{
    Interrupted = bValue;
}

#endif

constexpr int MAX_DEBUG_TELEGRAMS = 500;

#pragma comment(lib, "Iphlpapi.lib")

//------------------------------------------------------------------------------------------------------------------
/*
Supporting routines for the communication thread
*/
//------------------------------------------------------------------------------------------------------------------

bool Ping(const char *ipAddress, std::string &FeedBack, DWORD iTimeout)
{
    HANDLE            hIcmpFile;
    unsigned long     ipaddr     = INADDR_NONE;
    DWORD             dwRetVal   = 0;
    char              SendData[] = "Data Buffer";
    std::vector<char> ReplyBuffer;
    DWORD             ReplySize = 0;
    struct in_addr    addr;

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

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + 8;
    ReplyBuffer.resize(ReplySize);

    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), nullptr, ReplyBuffer.data(), ReplySize, iTimeout);
    if (dwRetVal != 0)
    {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer.data();
        struct in_addr   ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ReplyAddr, ipStr, INET_ADDRSTRLEN);
        FeedBack = "Received " + std::to_string(pEchoReply->DataSize) + " bytes from " + std::string(ipStr) +
                   ": icmp_seq=" + std::to_string(pEchoReply->RoundTripTime) + " ms";

        return true;
    }
    else
    {
        FeedBack = "Ping failed: " + std::to_string(GetLastError());
        return false;
    }
}

bool ResolveIP4_Address(const std::string &HostName, std::string &IP_Number)
{
    WSADATA wsaData;
    int     result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        PrintError("WSAStartup failed: {}", result);
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

        PrintError("getaddrinfo failed: {}", result);
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

bool IsTCPPortInUse(int port)
{
    WSADATA wsaData;
    int     result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        PrintError("WSAStartup failed: {}", result);
        return false;
    }
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        PrintError("Socket creation failed: {}", WSAGetLastError());
        WSACleanup();
        return false;
    }
    sockaddr_in service;
    service.sin_family      = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface
    service.sin_port        = htons(port);
    // Try binding the socket
    if (::bind(listenSocket, (SOCKADDR *)&service, sizeof(service)) == 0)
    {
        closesocket(listenSocket);
        WSACleanup();
        return false; // Port is available
    }
    else
    {
        int err = WSAGetLastError();
        closesocket(listenSocket);
        WSACleanup();
        if (err == WSAEADDRINUSE)
            return true; // Port is in use
        else
            PrintError("Bind failed with error: {}", err);
        return false; // Some other error occurred
    }
}

int FindAvailableTCPPortNumber(int startPort)
{
    WSADATA wsaData;
    int     result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        PrintError("WSAStartup failed: {}", result);
        return 0;
    }
    SOCKET      listenSocket = INVALID_SOCKET;
    sockaddr_in service;
    int         port = startPort;

    // Create a socket
    listenSocket     = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        PrintError("Socket creation: {}", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    // Setup the sockaddr_in structure
    service.sin_family      = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface

    // Try binding the socket starting from startPort
    for (; port <= 65535; ++port)
    {
        service.sin_port = htons(port);

        if (::bind(listenSocket, (SOCKADDR *)&service, sizeof(service)) == 0)
        {
            // Successfully bound to the port
            break;
        }
        else
        {
            int err = WSAGetLastError();
            if (err != WSAEADDRINUSE)
            {
                PrintError("Bind failed with error: {}", err);
                closesocket(listenSocket);
                WSACleanup();
                return 0;
            }
        }
    }

    // Cleanup
    closesocket(listenSocket);

    if (port > 65535)
    {
        PrintError("No free port found.");
        return 0;
    }

    WSACleanup();
    return port;
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
#ifdef _UNICODE
                std::wstring exeName = pe32.szExeFile;
                processName          = WstringToString(exeName);
#else
                processName = pe32.szExeFile;
#endif
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
    DWORD              dwSize   = 0;
    DWORD              dwRetVal = 0;
    std::vector<DWORD> returnVector;
    std::vector<char>  buffer;

    // Call GetExtendedTcpTable to get the size needed
    dwRetVal = GetExtendedTcpTable(nullptr, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    if (dwRetVal != ERROR_INSUFFICIENT_BUFFER)
    {
        PrintError("Failed to get buffer size for TCP table");
        return returnVector;
    }

    buffer.resize(dwSize);

    // Get the TCP table
    dwRetVal = GetExtendedTcpTable(reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data()), &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    if (dwRetVal != NO_ERROR)
    {
        PrintError("GetExtendedTcpTable failed with error: {}", dwRetVal);
        return returnVector;
    }

    PMIB_TCPTABLE_OWNER_PID pTcpTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data());

    // Loop through the TCP table
    for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++)
    {
        DWORD       pid         = pTcpTable->table[i].dwOwningPid;
        std::string processName = GetProcessNameByPID(pid);

        if (processName == appName)
        {
            DWORD localPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
            returnVector.push_back(localPort);
        }
    }

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

OnDiagnosticFunction PrintSendDiagnostics = [](std::unique_ptr<CTCPGram> &TCPGram, bool send, int port) -> void
{
    if (!TCPGram)
    {
        PrintError("Received empty TCPGram in PrintSendDiagnostics");
        return;
    }
    int code = TCPGram->GetCode();
    if (code == TCPGRAM_CODE_MESSAGE)
    {
        CTrack::Message message;
        if (TCPGram->GetMessage(message))
        {
            if (message.GetID() == EngineMsg::State)
            return;
            if (send)
            {
                std::string commandString = fmt::format(" [{}] Send message {} : {}", port, message.GetID(), message.GetParams().dump());
                PrintCommandReturn(commandString);
                LOG_DEBUG(commandString);
            }
            else
            {
                std::string commandString = fmt::format(" [{}] Received message {} : {}", port, message.GetID(), message.GetParams().dump());
                PrintCommand(commandString);
                LOG_DEBUG(commandString);
            }
        }
    }
    /*else
    {
        if (send)
            PrintCommandReturn(" [{}] Send TCPGram {} : {}", port,code, TCPGram->GetData().size());
        else
            PrintCommand(" [{}] Received TCPGram {} : {}",port, code);
    }*/
};

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationParameters
*/
//------------------------------------------------------------------------------------------------------------------

CCommunicationInterface::CCommunicationInterface()
{
    m_pMessageResponder = std::make_shared<CTrack::MessageResponder>();
    m_pMessageResponder->SetSendFunction([this](CTrack::Message &message) { SendMessage(message); });
}

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
        m_bDisableNagle     = ipFrom->m_bDisableNagle;
        m_TimeOut           = ipFrom->m_TimeOut;
    }
}

void CCommunicationInterface::SendMessage(CTrack::Message &message)
{
    std::unique_ptr<CTCPGram> tcpGram = std::make_unique<CTCPGram>(message);
    PushSendPackage(tcpGram);
}

void CCommunicationInterface::SendMessage(CTrack::Message &message, SOCKET destination)
{
    std::unique_ptr<CTCPGram> tcpGram = std::make_unique<CTCPGram>(message);
    tcpGram->SetDestination(destination);
    PushSendPackage(tcpGram);
}

CTrack::Subscription CCommunicationInterface::Subscribe(const std::string &messageID, CTrack::Handler handler)
{
    return m_pMessageResponder->Subscribe(messageID, handler);
}

bool CCommunicationInterface::GetSendPackage(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    std::lock_guard<std::mutex> Lock(m_sendMutex);
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
#ifdef _DEBUG
    while (m_arReceiveBuffer.size() > iNumberToKeep)
    {
        m_arReceiveBuffer.pop_front();
    }
#endif
}

bool CCommunicationInterface::GetReceivePackage(std::unique_ptr<CTCPGram> &TCPGram, unsigned char CodeFilter)
{
    std::lock_guard<std::mutex> Lock(m_receiveMutex);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif

    // go through the telegrams in the receive buffer until we find a valid telegram
    auto Iter = m_arReceiveBuffer.begin();
    while (Iter != m_arReceiveBuffer.end())
    {
        std::unique_ptr<CTCPGram> &pTCPGram = *Iter;

        // handle message telegrams on the fly as well
        if (pTCPGram->GetCode() == TCPGRAM_CODE_MESSAGE)
        {
            CTrack::Message message;
            if (pTCPGram->GetMessage(message))
            {
                m_pMessageResponder->RespondToMessage(message);
                Iter = m_arReceiveBuffer.erase(Iter);
            }
        }
        else if (CodeFilter == TCPGRAM_CODE_ALL || pTCPGram->GetCode() == CodeFilter)
        {
            TCPGram = std::move(pTCPGram);
            Iter    = m_arReceiveBuffer.erase(Iter);

            return true;
        }
        else
            Iter++;
    }
    return false;
}

void CCommunicationInterface::PushSendPackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    std::lock_guard<std::mutex> Lock(m_sendMutex);
    m_arSendBuffer.emplace_back(std::move(rTCPGram));
}

void CCommunicationInterface::PushReceivePackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    std::lock_guard<std::mutex> Lock(m_receiveMutex);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif
    m_arReceiveBuffer.emplace_back(std::move(rTCPGram));
}

void CCommunicationInterface::ClearBuffers()
{
    {
        std::lock_guard<std::mutex> Lock(m_sendMutex);
        m_arSendBuffer.clear();
    }
    {

        std::lock_guard<std::mutex> Lock(m_receiveMutex);
        m_arReceiveBuffer.clear();
    }
}

bool CCommunicationInterface::IsServer()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return (m_CommunicationMode == TCP_SERVER);
}

bool CCommunicationInterface::ErrorOccurred()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_bErrorOccurred;
}

std::string CCommunicationInterface::GetError()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_ErrorString;
}

void CCommunicationInterface::SetError(const std::string &iFileName, int iLineNumber, const std::string &iMessage)
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    m_bErrorOccurred  = true;
    m_ErrorString     = iMessage;
    m_ErrorSourceFile = iFileName;
    m_ErrorSourceLine = iLineNumber;
}

unsigned short CCommunicationInterface::GetPort()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_Port;
}

unsigned short CCommunicationInterface::GetPortUDP()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_PortUDP;
}

bool CCommunicationInterface::GetUDPBroadcast()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_bUDPBroadCast;
}

std::string CCommunicationInterface::GetHostName()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_HostName;
}

E_COMMUNICATION_Mode CCommunicationInterface::GetCommunicationMode()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_CommunicationMode;
}

void CCommunicationInterface::SetCommunicationMode(E_COMMUNICATION_Mode iMode)
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    m_CommunicationMode = iMode;
}

#ifdef CTRACK_UI
bool CCommunicationInterface::EditSettingsChanged()
{
    if (m_pPropertyPage.get())
    {
        return (m_CommunicationMode != static_cast<E_COMMUNICATION_Mode>(m_pPropertyPage->m_ComboDataType) || m_Port != m_pPropertyPage->m_Port ||
                m_bUDPBroadCast != m_pPropertyPage->m_bUDPBroadCast || m_HostName.c_str() != m_pPropertyPage->m_HostName ||
                m_PortUDP != m_pPropertyPage->m_PortUDP || m_bDisableNagle != m_pPropertyPage->m_bDisableNagle);
    }
    return false;
}

void CCommunicationInterface::EditSettings(CPropertySheet *pDialog)
{
    if (pDialog)
    {
        if (m_pPropertyPage.get() == nullptr)
            m_pPropertyPage.reset(new CDCommunicationPage);
        pDialog->AddPage(m_pPropertyPage.get());
        m_pPropertyPage->m_ComboDataType = static_cast<int>(m_CommunicationMode);
        m_pPropertyPage->m_Port          = m_Port;
        m_pPropertyPage->m_PortUDP       = m_PortUDP;
        m_pPropertyPage->m_bUDPBroadCast = m_bUDPBroadCast;
        m_pPropertyPage->m_HostName      = m_HostName.c_str();
        m_pPropertyPage->m_bDisableNagle = m_bDisableNagle;
    }
    else
    {
        if (m_pPropertyPage.get() != nullptr)
        {
            m_CommunicationMode = static_cast<E_COMMUNICATION_Mode>(m_pPropertyPage->m_ComboDataType);
            m_Port              = m_pPropertyPage->m_Port;
            m_PortUDP           = m_pPropertyPage->m_PortUDP;
            m_bUDPBroadCast     = m_pPropertyPage->m_bUDPBroadCast;
            m_HostName          = m_pPropertyPage->m_HostName;
            m_bDisableNagle     = m_pPropertyPage->m_bDisableNagle;
        }
    }
}
#endif

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
    m_Port              = STATEMANAGER_DEFAULT_TCP_PORT;
    m_HostName          = STATEMANAGER_DEFAULT_TCP_HOST;
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
    GetSetAttribute(pXML, ATTRIB_SEND_NAGLE, m_bDisableNagle, Read);
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

bool CCommunicationObject::Open(E_COMMUNICATION_Mode iTcpMode, int iPort, int iPortUDP, const std::string &iIpAddress)
{
    // start the communication thread
    m_CommunicationMode = iTcpMode;
    m_Port              = iPort;
    m_PortUDP           = iPortUDP;
    m_HostName          = iIpAddress;
    SetOnReceive(PrintSendDiagnostics);
    SetOnSend(PrintSendDiagnostics);
    if (!m_bOpened)
    {
        // when a communication object wants to open the communication channels, CCommunicationManager checks if there is already a thread running on the same
        // port, if yes then the communication object is added to the list of communication objects belonging to that port, if no, then we create a new thread

        int Port = GetPort();
        CCommunicationObject::AssignCommunicationThread(Port, *this);
        m_bOpened = true;
    }
    return m_bOpened;
}

void CCommunicationObject::Close()
{
    std::shared_ptr<CCommunicationThread> pCommunicationThread = m_pCommunicationThread.lock();
    if (pCommunicationThread)
    {
        int Port = GetPort();
        CCommunicationObject::CloseCommunicationThread(Port, *this);
    }
    m_bOpened = false;
}

CSocket *CCommunicationObject::SocketCreate(SOCKET iSocket, E_COMMUNICATION_Mode Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPBroadCastPort,
                                            bool UDPBroadcast, const std::string &UDPSendAddress)
{
    return new CSocket(iSocket, Mode, ipSockAddress, UDPBroadCastPort, UDPBroadcast, UDPSendAddress, m_bDisableNagle);
}

// In CCommunicationObject.cpp
void CCommunicationObject::SetCommunicationThread(std::shared_ptr<CCommunicationThread> &rCommunicationThread)
{
    // Use the general m_Mutex to protect modifications to members of CCommunicationInterface,
    // including m_pCommunicationThread and m_pMessageResponder.
    std::lock_guard<std::mutex> lock(m_infoMutex);

    m_pCommunicationThread = rCommunicationThread;
    if (m_ThreadName.size() > 0)
        rCommunicationThread->SetThreadName(m_ThreadName);
    std::shared_ptr<CCommunicationThread> pCommunicationThread = m_pCommunicationThread.lock();
    if (pCommunicationThread)
    {
        pCommunicationThread->SetMessageResponderInstance(this->m_pMessageResponder, this->m_OnReceiveFunction, this->m_OnSendFunction);
    }
}

size_t CCommunicationObject::GetNumConnections()
{
    std::shared_ptr<CCommunicationThread> pCommunicationThread = m_pCommunicationThread.lock();
    if (pCommunicationThread)
    {
        return pCommunicationThread->GetNumConnections();
    }
    return 0;
}

bool CCommunicationObject::WaitConnection(DWORD timeoutMs)
{
    std::shared_ptr<CCommunicationThread> pCommunicationThread = m_pCommunicationThread.lock();
    if (pCommunicationThread)
    {
        auto thread = pCommunicationThread.get();
        if (thread)
        {
            std::unique_lock<std::mutex> lock(thread->GetConnectionMutex());
            return thread->GetConnectionCV().wait_for(lock, std::chrono::milliseconds(timeoutMs), [thread] { return thread->GetNumConnections() > 0; });
        }
        else
        {
            // Fallback to polling
            DWORD startTick = GetTickCount();
            DWORD endTick   = startTick + timeoutMs;
            DWORD nowTick   = startTick;
            while (nowTick < endTick)
            {
                if (GetNumConnections() > 0)
                    return true;
                Sleep(50);
                nowTick = GetTickCount();
            }
            return false;
        }
    }
    return false;
}

void CCommunicationObject::PushSendPackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    std::shared_ptr<CCommunicationThread> pCommunicationThread = m_pCommunicationThread.lock();
    if (pCommunicationThread)
    {
        pCommunicationThread->PushSendPackage(rTCPGram);
    }
}

void CCommunicationObject::AssignCommunicationThread(UINT port, CCommunicationObject &pComObject)
{
    std::lock_guard<std::mutex>                                     lock(CCommunicationObject::getMapThreadsMutex());
    std::map<UINT /*port*/, std::shared_ptr<CCommunicationThread>> &maps = getMapThreads();
    std::shared_ptr<CCommunicationThread>                           pCommunicationThread;
    bool                                                            bStartThread = false;
    auto                                                            iterFind     = getMapThreads().find(port);
    if (iterFind == maps.end())
    {
        // not connected yet, create a CCommunicationThread object, copy parameters and start a thread
        pCommunicationThread.reset(new CCommunicationThread);
        pCommunicationThread->CopyFrom(&pComObject);
        maps[port]   = pCommunicationThread;
        bStartThread = true;
    }
    else
        pCommunicationThread = iterFind->second;

    if (pCommunicationThread)
    {
        pCommunicationThread->CommunicationObjectAdd(pComObject);
        pComObject.SetCommunicationThread(pCommunicationThread);
    }
    if (bStartThread)
        pCommunicationThread->StartThread();
}

void CCommunicationObject::CloseCommunicationThread(UINT port, CCommunicationObject &pComObject)
{
    std::lock_guard<std::mutex>                                     lock(CCommunicationObject::getMapThreadsMutex());
    std::map<UINT /*port*/, std::shared_ptr<CCommunicationThread>> &maps     = getMapThreads();
    auto                                                            iterFind = maps.find(port);
    if (iterFind != maps.end())
    {
        std::shared_ptr<CCommunicationThread> pCommunicationThread = iterFind->second;
        if (pCommunicationThread)
        {
            pCommunicationThread->CommunicationObjectRemove(pComObject);
            if (pCommunicationThread->CommunicationObjectGetNum() == 0)
            {
                pCommunicationThread->EndThread();
                maps.erase(iterFind);
            }
        }
    }
}

void CCommunicationObject::CloseAllConnections()
{
    std::lock_guard<std::mutex> lock(CCommunicationObject::getMapThreadsMutex());
    auto                        maps = getMapThreads();
    for (auto &[port, thread] : maps)
    {
        PrintDebug("Forcefully shutting down TCP communication for {}", port);
        if (thread)
            thread->EndThread();
    }
    maps.clear();
}

//------------------------------------------------------------------------------------------------------------------
/*
CSocket class
*/
//------------------------------------------------------------------------------------------------------------------

CSocket::CSocket(SOCKET iSocket, E_COMMUNICATION_Mode iCommunicationMode, SOCKADDR_IN *ipSockAddress, int UDBroadCastPort, bool iUDPBroadcast,
                 const std::string &iUDPSendToAddress, bool ibDisableNagle)
{
    m_Socket            = iSocket;
    m_CommunicationMode = iCommunicationMode;
    m_bDisableNagle     = ibDisableNagle;
    m_pSockAddress      = ipSockAddress;

    m_MaxUDPMessageSize = 0;

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
            DisableNagle(m_bDisableNagle);
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

void CSocket::Throw(const std::string &ErrorMessage)
{
    long    errval{0};
    int FAR optlen = 4;
    getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, (char *)&errval, &optlen);
    if (errval == WSAECONNRESET || errval == WSAENOTCONN || errval == WSAECONNABORTED || errval == 0)
        throw false; // connection was closed
    else
        CTRACK_THROW_SOCKET_ERROR(ErrorMessage, errval);
}

void CSocket::ResetBuffers()
{
    m_MessageHeader.Reset();   // header of the telegram
    m_ReceiveBuffer.Reset();   // buffer for receiving data
    m_bHeaderReceived = false; // if true then the header has been received
    m_Data.clear();            // data of the telegram
}

void SetSocketOption(SOCKET socket, int level, int option, int value, const std::string &errorMessage)
{
    if (setsockopt(socket, level, option, (const char *)&value, sizeof(value)) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        CTRACK_THROW_SOCKET_ERROR(errorMessage, LastError);
    }
}

void CSocket::DisableNagle(bool bDisableNagle)
{
    SetSocketOption(m_Socket, IPPROTO_TCP, TCP_NODELAY, bDisableNagle ? 1 : 0, "Failed to set the Nagle option");
}

void CSocket::SetReuseAddress(bool bEnableReuseAddress)
{
    SetSocketOption(m_Socket, SOL_SOCKET, SO_REUSEADDR, bEnableReuseAddress ? 1 : 0, "Failed to set the reuse address option");
}

void CSocket::SetBroadcast(bool bEnableBroadcast)
{
    SetSocketOption(m_Socket, SOL_SOCKET, SO_BROADCAST, bEnableBroadcast ? 1 : 0, "Failed to set the broadcast option");
}

void CSocket::SetNonBlocking(bool bNonBlocking)
{
    unsigned long argList[1];
    argList[0] = (bNonBlocking ? 0 : 1);
    if (ioctlsocket(m_Socket, FIONBIO, argList) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        CTRACK_THROW_SOCKET_ERROR(("Failed to set non-blocking option"), LastError);
    }
}

int CSocket::GetMaxUDPMessageSize()
{
    int maxSize = 0;
    int SizeOf  = sizeof(int);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&maxSize, &SizeOf) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        CTRACK_THROW_SOCKET_ERROR(("Failed to set the reuse address option"), LastError);
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
            CTRACK_THROW_SOCKET_ERROR("An error occurred on the TCP network", errval);
    }
    return false;
}

int CSocket::GetReadBufferSize()
{
    u_long bytesAvailable = 0;
    if (ioctlsocket(m_Socket, FIONREAD, &bytesAvailable) == SOCKET_ERROR)
    {
        Throw("Error getting read buffer size");
    }
    return static_cast<int>(bytesAvailable);
}

int CSocket::GetWriteBufferSize()
{
    DWORD idealSendBacklog = 0;
    DWORD bytesReturned    = 0;

    if (WSAIoctl(m_Socket, SIO_IDEAL_SEND_BACKLOG_QUERY, NULL, 0, &idealSendBacklog, sizeof(idealSendBacklog), &bytesReturned, NULL, NULL) == SOCKET_ERROR)
    {
        Throw("Error getting write buffer size");
    }

    return static_cast<int>(idealSendBacklog);
}

void CSocket::GetSocketBufferSizes(int &recvBufSize, int &sendBufSize)
{
    int recvSize = sizeof(recvBufSize);
    int sendSize = sizeof(sendBufSize);

    if (getsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char *)&recvBufSize, &recvSize) == SOCKET_ERROR)
    {
        Throw("Error getting receive buffer size");
    }

    if (getsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char *)&sendBufSize, &sendSize) == SOCKET_ERROR)
    {
        Throw("Error getting send buffer size");
    }
}

void CSocket::SetSocketBufferSizes(int newRecvSize, int newSendSize)
{
    if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char *)&newRecvSize, sizeof(newRecvSize)) == SOCKET_ERROR)
        PrintError("Error setting receive buffer size: {}", WSAGetLastError());

    if (setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char *)&newSendSize, sizeof(newSendSize)) == SOCKET_ERROR)
        PrintError("Error setting send buffer size: {}", WSAGetLastError());
}

bool SplitByDelimiter(const std::vector<char> &Original, const std::vector<char> &Delimiter, std::vector<char> &Extracted, std::vector<char> &Remaining)
{
    auto it = std::search(Original.begin(), Original.end(), Delimiter.begin(), Delimiter.end());
    if (it != Original.end())
    {
        Extracted.assign(Original.begin(), it + Delimiter.size());
        Remaining.assign(it + Delimiter.size(), Original.end());
        return true;
    }
    return false;
}

int CSocket::TCPReceiveChunk(TReceiveBuffer &context, bool block)
{
    int rNumReceived = 0;
    while (context.m_BytesLeft > 0)
    {
        rNumReceived = recv(m_Socket, context.m_pBuffer, context.m_BytesLeft, 0);

        if ((rNumReceived == SOCKET_ERROR) || (rNumReceived == 0))
        {
            int LastError = WSAGetLastError();
            if (LastError == WSAEWOULDBLOCK)
            {
                if (block)
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for a short while (10 ms)
                else
                    return 0; // Non-blocking mode, return with partial data
            }
            else
                Throw("An error occurred trying to read data from the network");
        }
        context.m_BytesLeft -= rNumReceived;
        context.m_pBuffer += rNumReceived;
        if (!block)
            return rNumReceived; // Non-blocking mode, return with partial data
    }
    return rNumReceived;
}

bool CSocket::ReadExtractTelegram(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    if (!DataAvailable())
        return false;

    if (m_bUseHeader)
    {
        // Receive the header first
        if (!m_bHeaderReceived)
        {
            if (!m_ReceiveBuffer.IsInitialized())
            {
                m_ReceiveBuffer.Reset(m_MessageHeader.GetData(), m_MessageHeader.GetHeaderSize());
            }
            TCPReceiveChunk(m_ReceiveBuffer, false);
            if (m_ReceiveBuffer.IsComplete())
            {
                m_bHeaderReceived = true;
                m_Data.resize(m_MessageHeader.GetPayloadSize());
                m_ReceiveBuffer.Reset(m_Data.data(), m_MessageHeader.GetPayloadSize());
            }
        }
        else
        {
            // Receive the message
            TCPReceiveChunk(m_ReceiveBuffer, false);
            if (m_ReceiveBuffer.IsComplete())
            {
                ReturnTCPGram.reset(new CTCPGram(m_MessageHeader, m_Data));
                ResetBuffers();
                return true;
            }
        }
    }
    else // No header, just receive the message
    {
        std::vector<char> chunkBuffer(1024);
        if (!m_ReceiveBuffer.IsInitialized())
        {
            m_Data.clear();
            m_ReceiveBuffer.Reset(chunkBuffer.data(), chunkBuffer.size());
        }

        int NumReceived = TCPReceiveChunk(m_ReceiveBuffer, false);
        if (NumReceived > 0)
        {
            m_Data.insert(m_Data.end(), chunkBuffer.begin(), chunkBuffer.begin() + NumReceived);

            std::vector<char> Extracted;
            std::vector<char> Remaining;
            if (SplitByDelimiter(m_Data, m_Delimiter, Extracted, Remaining))
            {
                ReturnTCPGram.reset(new CTCPGram(Extracted));
                m_Data = Remaining;
                return true;
            }
        }
    }
    return false;
}

void CSocket::TCPSendChunk(const char *pBuffer, int BufferSize)
{
    int NumBytesWritten = send(m_Socket, pBuffer, BufferSize, 0);
    if (NumBytesWritten != SOCKET_ERROR)
    {
        assert(NumBytesWritten == BufferSize);
    }
    else
    {
        Throw("An error occurred trying to send data over the TCP network");
    }
}

bool CSocket::WriteSendTelegram(std::unique_ptr<CTCPGram> &rTCPGram)
{
    if (m_bBlockWrite)
        return true;
    switch (m_CommunicationMode)
    {
        case TCP_CLIENT:
        case TCP_SERVER:
        {
            // write header
            if (m_bUseHeader)
            {
                TCPSendChunk(reinterpret_cast<const char *>(rTCPGram->m_MessageHeader.GetData()), rTCPGram->m_MessageHeader.GetHeaderSize());
            }
            TCPSendChunk(reinterpret_cast<const char *>(rTCPGram->m_Data.data()), static_cast<int>(rTCPGram->m_Data.size()));
            if (!m_bUseHeader)
            {
                if (m_Delimiter.size() > 0)
                    TCPSendChunk(reinterpret_cast<const char *>(m_Delimiter.data()), static_cast<int>(m_Delimiter.size()));
            }
        };
        break;
        case UDP:
        {
            if (rTCPGram->m_Data.size() > m_MaxUDPMessageSize)
            {
                std::string ErrorMessage =
                    fmt::format("Error sending data over UDP : the package size {} is bigger than allowed {}", rTCPGram->m_Data.size(), m_MaxUDPMessageSize);
                CTRACK_THROW_ERROR(ErrorMessage);
            }
            int rVal = ::sendto(m_Socket, reinterpret_cast<const char *>(rTCPGram->m_Data.data()), static_cast<int>(rTCPGram->m_Data.size()), 0,
                                (SOCKADDR *)&m_UDPBroadCastAddr, sizeof(m_UDPBroadCastAddr));
            if (rVal == SOCKET_ERROR)
            {
                Throw("An error occurred trying to send data over the UDP network");
            }
            else
                return (rVal == rTCPGram->m_Data.size());
        };
        break;
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------------
/*
CCommunicationThread
*/
//------------------------------------------------------------------------------------------------------------------

CCommunicationThread::CCommunicationThread()
{
    m_pMessageResponder = nullptr;
    m_bErrorOccurred    = false;
    m_CommunicationMode = TCP_SERVER;
    m_Port              = STATEMANAGER_DEFAULT_TCP_PORT;
    m_HostName          = STATEMANAGER_DEFAULT_TCP_HOST;
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    m_IterCurrentSocket = m_arSockets.begin();
}

CCommunicationThread::~CCommunicationThread()
{
    // The list m_arSockets will be implicitly destructed here.
    // If SocketDeleteAll() hasn't been called, or if it failed, this is where it might crash.
    int NumSockets = static_cast<int>(m_arSockets.size());
    m_arSockets.clear();
}

void CCommunicationThread::SetQuit(bool ibQuit)
{
    m_bQuit = ibQuit;
}

bool CCommunicationThread::GetQuit()
{
    return m_bQuit;
}

void CCommunicationThread::SetMessageResponderInstance(std::shared_ptr<CTrack::MessageResponder> responder, OnDiagnosticFunction &onReceiveFunction,
                                                       OnDiagnosticFunction &onSendFunction)
{
    std::lock_guard<std::mutex> lock(m_infoMutex); // Protect assignment with CCommunicationInterface's mutex
    if (m_pMessageResponder)
        ClearSubscriptions(); // Unsubscribe from the old responder to avoid dangling subscriptions
    m_pMessageResponder = responder;
    m_OnReceiveFunction = onReceiveFunction;
    m_OnSendFunction    = onSendFunction;
    // If other objects are already attached to this thread and were using an old responder,
    // they would ideally need to be updated. This scenario suggests that changing
    // a thread's established responder is complex and should be handled with care.
    // For this model, we primarily rely on this for initial setup if thread starts with nullptr.
}

void CCommunicationThread::CommunicationObjectAdd(CCommunicationObject &rCommunicationObject)
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    m_setCommunicationObject.insert(&rCommunicationObject);
    m_Port                 = rCommunicationObject.GetPort();
    m_PortUDP              = rCommunicationObject.GetPortUDP();
    m_OnConnectFunction    = rCommunicationObject.m_OnConnectFunction;
    m_OnDisconnectFunction = rCommunicationObject.m_OnDisconnectFunction;
}

void CCommunicationThread::CommunicationObjectRemove(CCommunicationObject &rCommunicationObject)
{
    { // leave these brackets, otherwise a deadlock will occur on closing
        std::lock_guard<std::mutex> Lock(m_infoMutex);
        m_setCommunicationObject.erase(&rCommunicationObject);
    }
    if (CommunicationObjectGetNum() == 0)
    {
        EndThread();
    }
}

size_t CCommunicationThread::CommunicationObjectGetNum()
{
    std::lock_guard<std::mutex> Lock(m_infoMutex);
    return m_setCommunicationObject.size();
}

void CCommunicationThread::SocketAdd(SOCKET iSocket, E_COMMUNICATION_Mode Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPBroadCastPort, bool UDPBroadcast,
                                     const std::string &UDPSendPort)
{
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    auto                        iter = m_setCommunicationObject.begin();
    if (iter != m_setCommunicationObject.end())
    {
        auto pNewSocket = std::unique_ptr<CSocket>((*iter)->SocketCreate(iSocket, Mode, ipSockAddress, UDPBroadCastPort, UDPBroadcast, UDPSendPort));
        m_arSockets.emplace_back(std::move(pNewSocket));
    }
    else
        assert(false);

    // Notify waiting threads about the new connection
    {
        std::lock_guard<std::mutex> cvLock(m_connectionMutex);
        m_connectionCV.notify_all();
    }
}

CSocket *CCommunicationThread::SocketFirst()
{
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    m_IterCurrentSocket = m_arSockets.begin();
    if (m_IterCurrentSocket != m_arSockets.end())
        return (m_IterCurrentSocket->get());
    else
        return nullptr;
}

CSocket *CCommunicationThread::SocketNext()
{
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    m_IterCurrentSocket++;
    if (m_IterCurrentSocket != m_arSockets.end())
        return (m_IterCurrentSocket->get());
    else
        return nullptr;
}

CSocket *CCommunicationThread::SocketDeleteCurrent()
{
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    if (m_IterCurrentSocket != m_arSockets.end())
    {
        m_IterCurrentSocket->release();
        m_IterCurrentSocket = m_arSockets.erase(m_IterCurrentSocket);

        // Notify waiting threads about the connection change
        {
            std::lock_guard<std::mutex> cvLock(m_connectionMutex);
            m_connectionCV.notify_all();
        }

        if (m_IterCurrentSocket != m_arSockets.end())
            return (m_IterCurrentSocket->get());
    }
    return nullptr;
}

void CCommunicationThread::SocketDeleteAll()
{
    {
        std::lock_guard<std::mutex> Lock(m_socketMutex);
        for (auto &arSocket : m_arSockets)
        {
            if (arSocket)
                arSocket.release();
        }
        m_arSockets.clear();
    }

    // Notify waiting threads about the connection change
    {
        std::lock_guard<std::mutex> cvLock(m_connectionMutex);
        m_connectionCV.notify_all();
    }
}

size_t CCommunicationThread::GetNumConnections()
{
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    return m_arSockets.size();
}

void CCommunicationThread::PushReceivePackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    // copy incoming telegrams to all CCommunicationObjects
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    for (auto iter : m_setCommunicationObject)
    {
        std::unique_ptr<CTCPGram> CopyTCPGram = std::make_unique<CTCPGram>();
        CopyTCPGram->CopyFrom(rTCPGram);
        iter->PushReceivePackage(CopyTCPGram);
    }
}

void CCommunicationThread::SetError(const std::string &iFileName, int iLineNumber, const std::string &iMessage)
{
    // copy error to all CCommunicationObjects
    std::lock_guard<std::mutex> Lock(m_socketMutex);
    PrintError("error occurred in {} at {} : {} ", iFileName.c_str(), iLineNumber, iMessage.c_str());
    for (auto iter : m_setCommunicationObject)
        iter->SetError(iFileName, iLineNumber, iMessage);
}

void CCommunicationThread::StartThread()
{
    if (!m_Thread.joinable())
    {
        m_Thread = std::thread(&CCommunicationThread::ThreadFunction, this);
        if (m_ThreadName.empty())
            m_ThreadName = fmt::format("CCommunicationThread_{}", GetPort());
        ThreadSetName(m_Thread, m_ThreadName.c_str());
    }
}

void CCommunicationThread::EndThread()
{
    SetQuit(true);
    if (m_Thread.joinable())
    {
        m_Thread.join();
    }
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
    std::string          IP4;
    std::string          HostName;
    unsigned short       PortNumber;
    unsigned short       PortNumberUDP;
    bool                 bUDPBroadcast;
    bool                 bContinueBigLoop = true;

    try
    {
        SetQuit(false);

        //--------------------------------------------------------------------------------------------------------
        // Initialize socket library
        //--------------------------------------------------------------------------------------------------------
        sockVersion = MAKEWORD(2, 2);
        // start dll
        if (WSAStartup(sockVersion, &wsaData) != 0)
            CTRACK_THROW_ERROR(("Failed to initialize the socket library"));

        //--------------------------------------------------------------------------------------------------------
        // Resolve IP4 address (no need for IP6)
        //--------------------------------------------------------------------------------------------------------
        HostName      = GetHostName();
        PortNumber    = GetPort();
        PortNumberUDP = GetPortUDP();
        bUDPBroadcast = GetUDPBroadcast();

        if (!ResolveIP4_Address(HostName, IP4))
        {
            std::string ErrorMessage = fmt::format("The host {} could not be resolved", HostName);
            CTRACK_THROW_ERROR(ErrorMessage);
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
                    std::string ErrorMessage = fmt::format("The creation of the socket (host : {} port:{}) failed.", HostName, PortNumber);
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                };
                PrintInfo("TCP client on port {}", PortNumber);
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
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // make it NON BLOCKING
                unsigned long argList[1];
                argList[0] = 1;
                int rVal   = ioctlsocket(MainSocket, FIONBIO, argList);
                if (rVal == SOCKET_ERROR)
                {
                    int         LastError    = WSAGetLastError();
                    std::string ErrorMessage = fmt::format("The creation of the socket (host : {} port:{}) failed.", HostName.c_str(), PortNumber);
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The binding of the socket (host : {} port:%d) failed.Lasterror was {}.\nPossibly the port is already in use.",
                                    HostName.c_str(), PortNumber, LastError);
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
                //
                // start listening
                if (::listen(MainSocket, 1 /*backlog amount permitted*/) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The listen command on the socket (host : {} port:{}) failed.Lasterror was {}.\nPossibly the port is already in use.",
                                    HostName.c_str(), PortNumber, LastError);
                    PrintError(ErrorMessage);
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
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
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage =
                        fmt::format("The binding of the socket (host : {} port:{}) failed.Lasterror was {}.\nPossibly the port is already in use.",
                                    HostName.c_str(), PortNumberUDP, LastError);
                    PrintError(ErrorMessage);
                    CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
                SocketAdd(MainSocket, UDP, &sincontrol, PortNumber, bUDPBroadcast, HostName);
                PrintInfo("UDP available on port {}", PortNumber);
            };
            break;
        }
    }
    catch (const std::exception &e)
    {
        SetError(__FILE__, __LINE__, e.what());
        bContinueBigLoop = false;
    }
#ifdef CTRACK
    catch (CException *e)
    {
        TCHAR ErrorMessage[2001];
        e->GetErrorMessage(ErrorMessage, 2000);
        SetError("unknown", 0, ErrorMessage);
        e->Delete();
        bContinueBigLoop = false;
    }
#endif
    catch (...)
    {
        SetError(__FILE__, __LINE__, "An unknown error occurred");
        bContinueBigLoop = false;
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
    while (bContinueBigLoop)
    {
        try
        {
            if (GetQuit())
                bContinueBigLoop = false;
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
                            PrintInfo("Client accepted at port {}", PortNumber);
                            SocketAdd(ClientSocket, TCP_SERVER, &sincontrol, 0, false, (""));
                            if (m_OnConnectFunction)
                                m_OnConnectFunction(ClientSocket, GetNumConnections());
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
                                CTRACK_THROW_SOCKET_ERROR(ErrorMessage, LastError);
                            }
                            ZeroMemory(&sincontrol, sizeof(sincontrol));
                            sincontrol.sin_family = PF_INET;
                            sincontrol.sin_port   = htons(PortNumber);
                            inet_pton(AF_INET, IP4.c_str(), &(sincontrol.sin_addr.s_addr));
                        }
                        if (connect(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) != SOCKET_ERROR)
                        {
                            PrintInfo("TCP client connected to {} on port {}", HostName, PortNumber);
                            SocketAdd(MainSocket, TCP_CLIENT, &sincontrol, 0, false, (""));
                            if (m_OnConnectFunction)
                                m_OnConnectFunction(MainSocket, GetNumConnections());
                        }
                        else
                        {
                            int SocketError = WSAGetLastError();
                            if ((SocketError != WSAECONNREFUSED) && (SocketError != WSAEWOULDBLOCK) && (SocketError != WSAEALREADY))
                                CTRACK_THROW_SOCKET_ERROR("An error occurred trying to connect to the server", SocketError);
                            else
                                ::Sleep(1000); // don't check every microsecond
                        }
                    }
                };
                break;
            }

            //--------------------------------------------------------------------------------------------------------
            // Sending data : loop over send data buffer, loop over connections until complete telegram is send, then
            // get the next telegram, until all telegrams have been send if sending is not possible because of closed
            // connection, remove the socket from arConnectionSocket
            //--------------------------------------------------------------------------------------------------------
            std::unique_ptr<CTCPGram> TCPGram;
            bool                      bAvailable = GetSendPackage(TCPGram);
            while (bAvailable)
            {
                bool bAllSocketsCompleted = true;
                if (TCPGram)
                {
                    if (m_OnSendFunction)
                    {
                        m_OnSendFunction(TCPGram, true, PortNumber);
                    };
                    SOCKET   Destination    = TCPGram->GetDestination();
                    CSocket *pCurrentSocket = SocketFirst();
                    while (pCurrentSocket != nullptr)
                    {
                        try
                        {
                            if (Destination == ALL_DESTINATIONS || Destination == pCurrentSocket->GetSocket())
                                if (!pCurrentSocket->WriteSendTelegram(TCPGram)) // returns false if sending of the telegram was not completed yet
                                    bAllSocketsCompleted = false;
                            pCurrentSocket = SocketNext();
                        }
                        catch (bool &) // disconnected, other exceptions are handled by outer routines
                        {
                            pCurrentSocket = SocketDeleteCurrent();
                            if (pCurrentSocket)
                            {
                                pCurrentSocket->ResetBuffers();
                                PrintWarning("TCP client disconnected from {} on port {}", HostName, PortNumber);
                                if (m_OnDisconnectFunction)
                                    m_OnDisconnectFunction(pCurrentSocket->GetSocket(), GetNumConnections());
                                if (CommunicationMode == TCP_CLIENT)
                                {
                                    PrintWarning("TCP client disconnected from ", HostName, " on port ", PortNumber);
                                    MainSocket = INVALID_SOCKET;
                                }
                            }
                        }
                    }
                }
                if (bAllSocketsCompleted) // get the next package
                {
                    bAvailable = GetSendPackage(TCPGram);
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
                    std::unique_ptr<CTCPGram> TCPGram;
                    while (pCurrentSocket->ReadExtractTelegram(TCPGram))
                    {
                        if (m_OnReceiveFunction)
                            m_OnReceiveFunction(TCPGram, false, PortNumber);

                        if (TCPGram->GetCode() == TCPGRAM_CODE_MESSAGE)
                        {
                            CTrack::Message message;
                            if (TCPGram->GetMessage(message))
                                m_pMessageResponder->RequestSetPromiseThread(message);
                        }

                        if (TCPGram->GetCode() == TCPGRAM_CODE_INTERRUPT)
                            InterruptSet(true);
                        else
                            PushReceivePackage(TCPGram);
                    }
                    pCurrentSocket = SocketNext();
                }
                catch (bool &) // disconnected, other exceptions are handled by outer routines
                {
                    SOCKET socket  = 0;
                    pCurrentSocket = SocketDeleteCurrent();
                    if (pCurrentSocket)
                    {
                        socket = pCurrentSocket->GetSocket();
                        pCurrentSocket->ResetBuffers();
                    }
                    PrintWarning("TCP client disconnected from {} on port {}", HostName, PortNumber);
                    if (m_OnDisconnectFunction)
                        m_OnDisconnectFunction(socket, GetNumConnections());
                    if (CommunicationMode == TCP_CLIENT)
                    {
                        MainSocket = INVALID_SOCKET;
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            SetError(__FILE__, __LINE__, e.what());
        }
#ifdef CTRACK
        catch (CException *e)
        {
            TCHAR ErrorMessage[2001];
            e->GetErrorMessage(ErrorMessage, 2000);
            SetError("unknown", 0, ErrorMessage);
            e->Delete();
        }
#endif
        catch (...)
        {
            SetError(__FILE__, __LINE__, "An unknown error occurred");
        }
    }
    PrintDebug("Closing thread for port {}", PortNumber);

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
