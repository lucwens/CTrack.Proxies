#include "TCPCommunication.h"
#include "TinyXML_Extra.h"
#include "ErrorHandler.h"
#include "TinyXML_AttributeValues.h"
#include <Ws2tcpip.h>
#include <Icmpapi.h>
#include <algorithm>
#pragma comment(lib, "Iphlpapi.lib")

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
void ThreadSetName(std::thread &rThread, LPCSTR iThreadName)
{
    CStringW WideNameString(iThreadName);
    HRESULT  r = ::SetThreadDescription(rThread.native_handle(), WideNameString);
}

bool Ping(const char *HostName, CStringA &FeedBack, DWORD iTimeout)
{
    HANDLE           hIcmpFile    = INVALID_HANDLE_VALUE;
    unsigned long    ipaddr       = INADDR_NONE;
    char             SendData[32] = "Data Buffer";
    LPVOID           ReplyBuffer  = nullptr;
    DWORD            ReplySize    = 0;
    PICMP_ECHO_REPLY pEchoReply   = nullptr;
    CStringA         IP_Number;

    if (!ResolveIP4_Address(HostName, IP_Number))
    {
        FeedBack.Format(("Ping failed : %s could not be resolved"), HostName);
        return false;
    }

    ipaddr = inet_addr(IP_Number);
    if (ipaddr == INADDR_NONE)
        return false;

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE)
    {
        FeedBack.Format("IcmpCreatefile returned error: %ld", GetLastError());
        return false;
    }

    ReplySize   = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = new char[ReplySize];
    if (ReplyBuffer == NULL)
    {
        FeedBack.Format("Unable to allocate memory", GetLastError());
        return false;
    }

    bool bReturn           = false;
    auto LambdaGetFeedback = [&](int Code)
    {
        switch (Code)
        {
            case IP_SUCCESS:
            {
                FeedBack.Format(("Ping to %s [%s] successfull with response time of %ld ms."), HostName, IP_Number, pEchoReply->RoundTripTime);
                bReturn = true;
            };
            break;
            case IP_REQ_TIMED_OUT:
                FeedBack.Format(("Ping to %s failed: The request timed out."), HostName);
                ;
                break;
            case IP_DEST_NET_UNREACHABLE:
                FeedBack.Format(("Ping to %s failed: The destination network was unreachable."), HostName);
                ;
                break;
            case IP_DEST_HOST_UNREACHABLE:
                FeedBack.Format(("Ping to %s failed: The destination host was unreachable."), HostName);
                ;
                break;
            case IP_DEST_PROT_UNREACHABLE:
                FeedBack.Format(("Ping to %s failed: The destination protocol was unreachable."), HostName);
                ;
                break;
            case IP_DEST_PORT_UNREACHABLE:
                FeedBack.Format(("Ping to %s failed: The destination port was unreachable."), HostName);
                ;
                break;
            case IP_TTL_EXPIRED_TRANSIT:
                FeedBack.Format(("Ping to %s failed: The time to live (TTL) expired in transit."), HostName);
                ;
                break;
            case IP_TTL_EXPIRED_REASSEM:
                FeedBack.Format(("Ping to %s failed: The time to live expired during fragment reassembly."), HostName);
                ;
                break;
            case IP_BAD_DESTINATION:
                FeedBack.Format(("Ping to %s failed: A bad destination."), HostName);
                ;
                break;
            default:
                FeedBack.Format(("Ping to %s failed with error %d."), HostName, pEchoReply->Status);
                break;
        }
    };

    DWORD dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, iTimeout /*msec timeout*/);
    if (dwRetVal != 0)
    {
        pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        if (pEchoReply->Status == IP_SUCCESS)
        {
            FeedBack.Format(("Ping to %s [%s] successfull with response time of %ld ms."), HostName, IP_Number, pEchoReply->RoundTripTime);
            bReturn = true;
        }
        else
            LambdaGetFeedback(pEchoReply->Status);
    }
    else
        LambdaGetFeedback(GetLastError());

    if (hIcmpFile != INVALID_HANDLE_VALUE)
        IcmpCloseHandle(hIcmpFile);

    if (ReplyBuffer != nullptr)
        delete[] ReplyBuffer;
    return bReturn;
}

bool ResolveIP4_Address(LPCSTR HostName, CStringA &IP_Number)
{
    // local host spotted, return true
    IP_Number = HostName;
    if (IP_Number.CompareNoCase(("127.0.0.1")) == 0)
        return true;

    // ip of xxx.xxx.xxx.xxx or xxx.xxx.xxx.xxx.xxx.xxx spotted
    int IPNumbers[6];
    int NumScanned = sscanf(HostName, "%d.%d.%d.%d.%d.%d", &IPNumbers[0], &IPNumbers[1], &IPNumbers[2], &IPNumbers[3], &IPNumbers[4], &IPNumbers[5]);
    if (NumScanned == 4)
    {
        IP_Number = HostName;
        return true;
    }

    // something like "MyComputer" or "www.test.com" spotted
    addrinfo *res = nullptr;
    addrinfo  hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family         = AF_INET;     // we only look for IP4 addresses here, otherwise specify AF_UNSPEC;
    hints.ai_socktype       = SOCK_STREAM; // only TCP supported, other options SOCK_DGRAM SOCK_RAW
    hints.ai_protocol       = IPPROTO_TCP; //

    int Result              = getaddrinfo(HostName, nullptr, /*&hints*/ nullptr, &res);

    struct addrinfo    *ptr = nullptr;
    struct sockaddr_in *sockaddr_ipv4;
    int                 i       = 1;
    int                 iRetval = 0;
    bool                bFound  = false;
    for (ptr = res; ptr != nullptr; ptr = ptr->ai_next)
    {
        if (ptr->ai_family == AF_INET)
        {
            sockaddr_ipv4 = (struct sockaddr_in *)ptr->ai_addr;
            IP_Number     = inet_ntoa(sockaddr_ipv4->sin_addr);
            bFound        = true;
        }
    }
    freeaddrinfo(res);
    return bFound;
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
    CSingleLock Lock(&m_Lock, true);
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
    CSingleLock Lock(&m_Lock, true);
    while (m_arReceiveBuffer.size() > iNumberToKeep)
    {
        m_arReceiveBuffer.pop_front();
    }
}

bool CCommunicationInterface::GetReceivePackage(std::unique_ptr<CTCPGram> &ReturnTCPGram, unsigned char Code)
{
    CSingleLock Lock(&m_Lock, true);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif
    if (!m_arReceiveBuffer.empty())
    {
        // iterate from front to back, check for valid code
        auto &Iter = m_arReceiveBuffer.begin();
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
    CSingleLock Lock(&m_Lock, true);
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
    CSingleLock Lock(&m_Lock, true);
    m_arSendBuffer.emplace_back(std::move(rTCPGram));
}

void CCommunicationInterface::PushReceivePackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    CSingleLock Lock(&m_Lock, true);
#ifdef _DEBUG
    RemoveOldReceiveTelegrams(MAX_DEBUG_TELEGRAMS);
#endif
    m_arReceiveBuffer.emplace_back(std::move(rTCPGram));
}

void CCommunicationInterface::ClearBuffers()
{
    CSingleLock Lock(&m_Lock, true);
    m_arSendBuffer.clear();
    m_arReceiveBuffer.clear();
}

bool CCommunicationInterface::IsServer()
{
    CSingleLock Lock(&m_Lock, true);
    return (m_CommunicationMode == TCP_SERVER);
}

bool CCommunicationInterface::ErrorOccurred()
{
    CSingleLock Lock(&m_Lock, true);
    return m_bErrorOccurred;
}

CStringA CCommunicationInterface::GetError()
{
    CSingleLock Lock(&m_Lock, true);
    return m_ErrorString;
}

void CCommunicationInterface::SetError(LPCSTR iFileName, int iLineNumber, LPCSTR iMessage)
{
    CSingleLock Lock(&m_Lock, true);
    m_bErrorOccurred  = true;
    m_ErrorString     = iMessage;
    m_ErrorSourceFile = iFileName;
    m_ErrorSourceLine = iLineNumber;
}

unsigned short CCommunicationInterface::GetPort()
{
    CSingleLock Lock(&m_Lock, true);
    return m_Port;
}

unsigned short CCommunicationInterface::GetPortUDP()
{
    CSingleLock Lock(&m_Lock, true);
    return m_PortUDP;
}

bool CCommunicationInterface::GetUDPBroadcast()
{
    CSingleLock Lock(&m_Lock, true);
    return m_bUDPBroadCast;
}

CStringA CCommunicationInterface::GetHostName()
{
    CSingleLock Lock(&m_Lock, true);
    return m_HostName;
}

E_COMMUNICATION_Mode CCommunicationInterface::GetCommunicationMode()
{
    CSingleLock Lock(&m_Lock, true);
    return m_CommunicationMode;
}

void CCommunicationInterface::SetCommunicationMode(E_COMMUNICATION_Mode iMode)
{
    CSingleLock Lock(&m_Lock, true);
    m_CommunicationMode = iMode;
}

void CCommunicationInterface::AddNewComer(CSocket *iSocket)
{
    CSingleLock Lock(&m_Lock, true);
    m_arNewComers.push_back(iSocket);
}

CSocket *CCommunicationInterface::GetNewComer()
{
    CSingleLock Lock(&m_Lock, true);
    CSocket    *ReturnSocket = nullptr;
    auto        iter         = m_arNewComers.begin();
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

void CCommunicationObject::Open(E_COMMUNICATION_Mode iTcpMode, int iPort, int iPortUDP, LPCSTR iIpAddress)
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

        m_pCommunicationThread.reset(new CCommunicationThread);
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
    int NewNumConnections = GetNumConnections();
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
                cout << "Failed converting the node from XML" << endl;
            };
            break;
            case TCPGRAM_CODE_STATUS: // state changing is only allowed by the server, so normally we do not receive states here
            {
                std::unique_ptr<TiXmlElement> XML = Telegram->GetXML();
                assert(XML.get() != NULL);
                // 			CStringA StateName = StripName(XML->Value());
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
                                            bool UDPBroadcast, LPCSTR UDPSendAddress)
{
    return new CSocket(iSocket, Mode, ipSockAddress, UDPBroadCastPort, UDPBroadcast, UDPSendAddress);
}

int CCommunicationObject::GetNumConnections()
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
                 LPCSTR iUDPSendToAddress)
{
    m_Socket                 = iSocket;
    m_CommunicationMode      = iCommunicationMode;
    m_pSockAddress           = ipSockAddress;
    m_CurrentTelegramSize    = 0;
    m_pAccumulationBuffer    = nullptr;
    m_AccumulationBufferSize = 0;

    m_TotalBytesWritten      = 0;
    m_MaxUDPMessageSize      = 0;

    ZeroMemory(&m_UDPBroadCastAddr, sizeof(m_UDPBroadCastAddr));
    if (iUDPBroadcast)
        m_UDPBroadCastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    else
        m_UDPBroadCastAddr.sin_addr.s_addr = inet_addr(iUDPSendToAddress);

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
    if (m_pAccumulationBuffer)
        delete[] m_pAccumulationBuffer;
    m_pAccumulationBuffer = nullptr;
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

void CSocket::ReadFeedBuffer() // NumReceived
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
        AppendBack(m_pAccumulationBuffer, m_AccumulationBufferSize, ReceiveBuffer, rNumReceived);
}

bool CSocket::ReadExtractTelegram(std::unique_ptr<CTCPGram> &ReturnTCPGram)
{
    if (m_AccumulationBufferSize == 0)
        return false; // nothing in the buffer

    if (m_CurrentTelegramSize == 0 && m_pAccumulationBuffer != nullptr && m_AccumulationBufferSize >= TCPGRAM_HEADER_SIZE) // start of a new telegram
        m_CurrentTelegramSize = GetSize(m_pAccumulationBuffer);

    if (m_AccumulationBufferSize >= m_CurrentTelegramSize) // new telegram ready
    {
        ReturnTCPGram.reset(new CTCPGram(RemoveFront(
            m_pAccumulationBuffer, m_AccumulationBufferSize,
            m_CurrentTelegramSize))); // remove front returns the split of buffer which is then coupled directly with the telegram, who will finally destroy it
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
            int NumBytesWritten = send(m_Socket, rTCPGram->m_pData.get(), rTCPGram->m_PackageSize, 0);
            if (NumBytesWritten != SOCKET_ERROR)
            {
                m_TotalBytesWritten += NumBytesWritten;
                bool bFinished = (m_TotalBytesWritten == rTCPGram->m_PackageSize);
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
            if (rTCPGram->m_PackageSize > m_MaxUDPMessageSize)
            {
                CStringA ErrorMessage;
                ErrorMessage.Format(("Error sending data over UDP : the package size (%d) is bigger than allowed (%d)"), rTCPGram->m_PackageSize,
                                    m_MaxUDPMessageSize);
                THROW_ERROR(ErrorMessage);
            }
            int rVal = ::sendto(m_Socket, rTCPGram->m_pData.get(), rTCPGram->m_PackageSize, 0, (SOCKADDR *)&m_UDPBroadCastAddr, sizeof(m_UDPBroadCastAddr));
            if (rVal == SOCKET_ERROR)
            {
                int LastError = WSAGetLastError();
                THROW_SOCKET_ERROR(("An error occurred trying to send data over the UDP network"), LastError);
            }
            else
                return (rVal == rTCPGram->m_PackageSize);
        };
        break;
    }
    return false;
}

void CSocket::AppendBack(char *&pMainBuffer, unsigned long &MainBufferSize, char *pAppendBuffer, unsigned long NewBufferSize)
{
    if (NewBufferSize == 0)
        return;

    char *pNewBuffer = new char[MainBufferSize + NewBufferSize];
    if (MainBufferSize)
        memcpy(pNewBuffer, pMainBuffer, MainBufferSize);
    memcpy(&pNewBuffer[MainBufferSize], pAppendBuffer, NewBufferSize);
    if (pMainBuffer)
        delete[] pMainBuffer;
    MainBufferSize += NewBufferSize;
    pMainBuffer = pNewBuffer;
}

char *CSocket::RemoveFront(char *&pMainBuffer, unsigned long &MainBufferSize, unsigned long SizeToRemove)
{
    char *pReturnBuffer = nullptr;
    if (SizeToRemove > MainBufferSize || SizeToRemove == 0)
        return nullptr;
    if (MainBufferSize > SizeToRemove)
    {
        // copy front part to return pointer
        pReturnBuffer = new char[SizeToRemove];
        memcpy(pReturnBuffer, pMainBuffer, SizeToRemove);

        // remove from main buffer
        MainBufferSize -= SizeToRemove;
        char *pNewBuffer = new char[MainBufferSize];
        memcpy(pNewBuffer, &pMainBuffer[SizeToRemove], MainBufferSize);

        if (pMainBuffer)
            delete[] pMainBuffer;
        pMainBuffer = pNewBuffer;
    }
    else if (MainBufferSize == SizeToRemove)
    {
        pReturnBuffer  = pMainBuffer;
        pMainBuffer    = nullptr;
        MainBufferSize = 0;
    }
    return pReturnBuffer;
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
    CSingleLock Lock(&m_Lock, true);
    m_IterCurrentSocket = m_arSockets.begin();
}

CCommunicationThread::~CCommunicationThread()
{
#ifdef _DEBUG
    DebugPrint("Deleting communication thread for port : ", m_Port);
#endif
}

void CCommunicationThread::SetQuit(bool ibQuit)
{
#ifdef _DEBUG
    if (ibQuit)
        DebugPrint("Setting quit for port : ", m_Port);
#endif
    m_bQuit = ibQuit;
}

bool CCommunicationThread::GetQuit()
{
#ifdef _DEBUG
    if (m_bQuit)
        DebugPrint("Positive quit for port : ", m_Port);
#endif
    return m_bQuit;
}

void CCommunicationThread::CommunicationObjectAdd(CCommunicationObject &rCommunicationObject)
{
    CSingleLock Lock(&m_Lock, true);
    m_setCommunicationObject.insert(&rCommunicationObject);
    m_Port    = rCommunicationObject.GetPort();
    m_PortUDP = rCommunicationObject.GetPortUDP();
}

void CCommunicationThread::CommunicationObjectRemove(CCommunicationObject &rCommunicationObject)
{
    CSingleLock Lock(&m_Lock, true);
    m_setCommunicationObject.erase(&rCommunicationObject);
    if (CommunicationObjectGetNum() == 0)
    {
        Lock.Unlock();
        EndThread();
    }
}

int CCommunicationThread::CommunicationObjectGetNum()
{
    CSingleLock Lock(&m_Lock, true);
    return m_setCommunicationObject.size();
}

void CCommunicationThread::SocketAdd(SOCKET iSocket, E_COMMUNICATION_Mode Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPBroadCastPort,
                                     bool bAddToNewComerList, bool UDPBroadcast, LPCSTR UDPSendPort)
{
    CSingleLock Lock(&m_Lock, true);
    // a communication thread can have multiple ccommunicationObjects, which may have different SocketCreate methods, here we just take the first
    auto        iter = m_setCommunicationObject.begin();
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
    CSingleLock Lock(&m_Lock, true);
    m_IterCurrentSocket = m_arSockets.begin();
    if (m_IterCurrentSocket != m_arSockets.end())
        return (*m_IterCurrentSocket);
    else
        return nullptr;
}

CSocket *CCommunicationThread::SocketNext()
{
    CSingleLock Lock(&m_Lock, true);
    m_IterCurrentSocket++;
    if (m_IterCurrentSocket != m_arSockets.end())
        return (*m_IterCurrentSocket);
    else
        return nullptr;
}

CSocket *CCommunicationThread::SocketDeleteCurrent()
{
    CSingleLock Lock(&m_Lock, true);
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
    CSingleLock Lock(&m_Lock, true);
    for (auto &arSocket : m_arSockets)
        delete arSocket;
    m_arSockets.clear();
}

void CCommunicationThread::SocketResetSend()
{
    CSingleLock Lock(&m_Lock, true);
    for (auto &arSocket : m_arSockets)
        arSocket->WriteSendReset();
}

void CCommunicationThread::AddNewComer(CSocket *iSocket)
{
    CSingleLock Lock(&m_Lock, true);
    //
    // do not store locally, copy all to CCommunicationObjects
    for (auto iter : m_setCommunicationObject)
        iter->AddNewComer(iSocket);
}

int CCommunicationThread::GetNumConnections()
{
    CSingleLock Lock(&m_Lock, true);
    return m_arSockets.size();
}

void CCommunicationThread::PushReceivePackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    // copy incoming telegrams to all CCommunicationObjects
    CSingleLock Lock(&m_Lock, true);
    for (auto iter : m_setCommunicationObject)
    {
        std::unique_ptr<CTCPGram> CopyTCPGram = std::make_unique<CTCPGram>();
        CopyTCPGram->CopyFrom(rTCPGram);
        iter->PushReceivePackage(CopyTCPGram);
    }
}

void CCommunicationThread::SetError(LPCSTR iFileName, int iLineNumber, LPCSTR iMessage)
{
    // copy error to all CCommunicationObjects
    CSingleLock Lock(&m_Lock, true);
    for (auto iter : m_setCommunicationObject)
        iter->SetError(iFileName, iLineNumber, iMessage);
}

void CCommunicationThread::StartThread()
{
    if (!m_Thread.joinable())
    {
        m_Thread = std::thread(&CCommunicationThread::ThreadFunction, this);
        CStringA ThreadName(("CCommunicationThread"));
        if (!m_ThreadName.IsEmpty())
            ThreadName.Format(("CommThread_ %s"), m_ThreadName);
        ThreadSetName(m_Thread, ThreadName);
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
        CStringA       IP4;
        CStringA       HostName      = GetHostName();
        unsigned short PortNumber    = GetPort();
        unsigned short PortNumberUDP = GetPortUDP();
        bool           bUDPBroadcast = GetUDPBroadcast();

        if (!ResolveIP4_Address(HostName, IP4))
        {
            CStringA ErrorMessage;
            ErrorMessage.Format(("The host %s could not be resolved"), HostName);
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
                sincontrol.sin_family      = PF_INET;
                sincontrol.sin_port        = htons(PortNumber);
                sincontrol.sin_addr.s_addr = inet_addr(IP4);
                MainSocket                 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (MainSocket == INVALID_SOCKET)
                {
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumber);
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
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The creation of the socket (host : %s port:%d) failed. Lasterror was %d."), HostName, PortNumber, LastError);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // make it NON BLOCKING
                unsigned long argList[1];
                argList[0] = 1;
                int rVal   = ioctlsocket(MainSocket, FIONBIO, argList);
                if (rVal == SOCKET_ERROR)
                {
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumber);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The binding of the socket (host : %s port:%d) failed.Lasterror was %d.\nPossibly the port is already in use."),
                                        HostName, PortNumber, LastError);
                    DebugPrint(ErrorMessage);
                    CString wErrorMessage(ErrorMessage);
                    AfxMessageBox(wErrorMessage, MB_ICONERROR);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
                //
                // start listening
                if (::listen(MainSocket, 1 /*backlog amount permitted*/) == SOCKET_ERROR)
                {
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The listen command on the socket (host : %s port:%d) failed.Lasterror was %d.\nPossibly the port is already in use."),
                                        HostName, PortNumber, LastError);
                    std::cout << ErrorMessage << std::endl;
                    CString wErrorMessage(ErrorMessage);
                    AfxMessageBox(wErrorMessage, MB_ICONERROR);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }
                DebugPrint("TCP server listening on port ", PortNumber);
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
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumberUDP);
                    THROW_SOCKET_ERROR(ErrorMessage, LastError);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int      LastError = WSAGetLastError();
                    CStringA ErrorMessage;
                    ErrorMessage.Format(("The binding of the socket (host : %s port:%d) failed.Lasterror was %d.\nPossibly the port is already in use."),
                                        HostName, PortNumberUDP, LastError);
                    std::cout << ErrorMessage << std::endl;
                    CString wErrorMessage(ErrorMessage);
                    AfxMessageBox(wErrorMessage, MB_ICONERROR);
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
#ifdef _DEBUG
        DebugPrint("Starting thread for port : ", PortNumber);
#endif
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
                            SocketAdd(ClientSocket, TCP_SERVER, &sincontrol, 0, true, false, (""));
                            DebugPrint("TCP server accepting client on port ", PortNumber);
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
                                int      LastError = WSAGetLastError();
                                CStringA ErrorMessage;
                                ErrorMessage.Format(("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumber);
                                THROW_SOCKET_ERROR(ErrorMessage, LastError);
                            }
                            ZeroMemory(&sincontrol, sizeof(sincontrol));
                            sincontrol.sin_family      = PF_INET;
                            sincontrol.sin_port        = htons(PortNumber);
                            sincontrol.sin_addr.s_addr = inet_addr(IP4);
                        }
                        if (connect(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) != SOCKET_ERROR)
                        {
                            DebugPrint("TCP client connected to ", HostName, " on port ", PortNumber);
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
                                DebugPrint("TCP client disconnected from ", HostName, " on port ", PortNumber);
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
                    if (CommunicationMode == TCP_CLIENT)
                    {
                        DebugPrint("TCP client disconnected from ", HostName, " on port ", PortNumber);
                        MainSocket = INVALID_SOCKET;
                    }
                }
            }
        }
#ifdef _DEBUG
        DebugPrint("Closing thread for port : ", PortNumber);
#endif
    }
    catch (CExceptionSocket &e)
    {
        SetError(__FILE__, __LINE__, e.GetMessage());
    }
    catch (CException *e)
    {
        TCHAR ErrorMessage[2001];
        e->GetErrorMessage(ErrorMessage, 2000);
        CStringA ErrorString(ErrorMessage);
        SetError(__FILE__, __LINE__, ErrorString);
        e->Delete();
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
