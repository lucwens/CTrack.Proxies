#include "TCPCommunication.h"
#include "ErrorException.h"
#include "ProcessRoutines.h"
#include "MinMax.h"
#include "TinyXML_AttributeValues.h"
#include "MinMax.h"
#include <Ws2tcpip.h>
#include <Icmpapi.h>
#pragma comment(lib, "Iphlpapi.lib")

#define RECEIVEBUFFERLENGTH 65535
#define MAX_DEBUG_TELEGRAMS 150
char ReceiveBuffer[RECEIVEBUFFERLENGTH];

#define THROW_ERROR(a)                (throw gcnew System::Exception(gcnew System::String(a)))
#define THROW_SOCKET_ERROR(a, b)      (throw gcnew CNetworkException(gcnew System::String(a), b))
#define STATEMANAGER_DEFAULT_TCP_PORT 40004
#define STATEMANAGER_DEFAULT_TCP_HOST _T("localhost")

#define THROW_SOCKET_ERROR(a, b)      (throw CExceptionSocket(__FILE__, __LINE__, a, b))

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

unsigned long GetSize(char *pBuffer)
{
    unsigned long ReturnSize(0);
    if (pBuffer)
    {
        memcpy(&ReturnSize, &pBuffer[TCPGRAM_INDEX_SIZE], sizeof(unsigned long));
        return ReturnSize;
    }
    else
        return 0;
}

void SetSize(char *pBuffer, unsigned long iSize)
{
    memcpy(&pBuffer[TCPGRAM_INDEX_SIZE], &iSize, sizeof(unsigned long));
}

int GetCode(char *pBuffer)
{
    unsigned char ReturnCode(0);
    memcpy(&ReturnCode, &pBuffer[TCPGRAM_INDEX_CODE], sizeof(unsigned char));
    return ReturnCode;
}

void SetCode(char *pBuffer, int iCode)
{
    memcpy(&pBuffer[TCPGRAM_INDEX_CODE], &iCode, sizeof(unsigned char));
}

//------------------------------------------------------------------------------------------------------------------
/*
CTCPGRam class
*/
//------------------------------------------------------------------------------------------------------------------

CTCPGram::CTCPGram(char *pFromReadBuffer)
{
    m_Destination = ALL_DESTINATIONS;
    m_Source      = 0;
    m_PackageSize = ::GetSize(pFromReadBuffer);
    m_pData.reset(pFromReadBuffer);
}

CTCPGram::CTCPGram(char *pBytes, unsigned long NumBytes)
{
    m_Destination = ALL_DESTINATIONS;
    m_Source      = 0;
    m_PackageSize = NumBytes;
    m_pData.reset(new char[m_PackageSize]);
    memcpy(m_pData.get(), pBytes, m_PackageSize);
}

CTCPGram::CTCPGram(double *pDoubles, int NumDoubles)
{
    m_Destination = ALL_DESTINATIONS;
    m_Source      = 0;
    m_PackageSize = TCPGRAM_HEADER_SIZE + sizeof(double) * NumDoubles;
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), TCPGRAM_CODE_DOUBLES);
    SetSize(m_pData.get(), m_PackageSize);

    memcpy((&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD])), (void *)pDoubles, sizeof(double) * NumDoubles);
}

CTCPGram::CTCPGram(std::vector<double> &arDoubles)
{
    m_Destination  = ALL_DESTINATIONS;
    m_Source       = 0;
    int NumDoubles = arDoubles.size();
    m_PackageSize  = TCPGRAM_HEADER_SIZE + sizeof(double) * NumDoubles;
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), TCPGRAM_CODE_DOUBLES);
    SetSize(m_pData.get(), m_PackageSize);

    double *pDouble = (double *)&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD]);
    for (int c = 0; c < NumDoubles; c++)
        pDouble[c] = arDoubles[c];
}

CTCPGram::CTCPGram(cliext::vector<double> ^ arDoubles)
{
    m_Destination  = ALL_DESTINATIONS;
    m_Source       = 0;
    int NumDoubles = arDoubles->size();
    m_PackageSize  = TCPGRAM_HEADER_SIZE + sizeof(double) * NumDoubles;
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), TCPGRAM_CODE_DOUBLES);
    SetSize(m_pData.get(), m_PackageSize);

    double *pDouble = (double *)&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD]);
    for (int c = 0; c < NumDoubles; c++)
        pDouble[c] = arDoubles[c];
}

CTCPGram::CTCPGram(cliext::deque<double> ^ queDoubles)
{
    m_Destination  = ALL_DESTINATIONS;
    m_Source       = 0;
    int NumDoubles = queDoubles->size();
    m_PackageSize  = TCPGRAM_HEADER_SIZE + sizeof(double) * NumDoubles;
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), TCPGRAM_CODE_DOUBLES);
    SetSize(m_pData.get(), m_PackageSize);

    double *pDouble = (double *)&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD]);
    for (int c = 0; c < NumDoubles; c++)
        pDouble[c] = queDoubles[c];
}

CTCPGram::CTCPGram(std::string &rString, unsigned char Code)
{
    m_Destination = ALL_DESTINATIONS;
    m_Source      = 0;
    m_PackageSize = TCPGRAM_HEADER_SIZE + sizeof(TCHAR) * (rString.GetLength() + 1);
    m_pData.reset(new char[m_PackageSize]);
    SetCode(m_pData.get(), Code);
    SetSize(m_pData.get(), m_PackageSize);

    // transfer xml string
    m_pData.get()[TCPGRAM_INDEX_PAYLOAD] = _T('\0');
    strcat(&m_pData.get()[TCPGRAM_INDEX_PAYLOAD], (LPCTSTR)rString);
}

CTCPGram::CTCPGram(XMLElement *pCommand, unsigned char Code)
{
    if (pCommand)
    {
        std::string  XMLText;
        TiXmlPrinter printer;
        printer.SetIndent("\t");
        pCommand->Accept(&printer);
        XMLText       = printer.CStr();

        m_Destination = ALL_DESTINATIONS;
        m_Source      = 0;
        m_PackageSize = TCPGRAM_HEADER_SIZE + sizeof(TCHAR) * (XMLText.GetLength() + 1);

        m_pData.reset(new char[m_PackageSize]);
        SetCode(m_pData.get(), Code);
        SetSize(m_pData.get(), m_PackageSize);

        // transfer xml string
        m_pData.get()[TCPGRAM_INDEX_PAYLOAD] = _T('\0');
        strcat(&m_pData.get()[TCPGRAM_INDEX_PAYLOAD], (LPCTSTR)XMLText);
    }
}

void CTCPGram::CopyFrom(std::unique_ptr<CTCPGram> &rFrom)
{
    m_PackageSize = rFrom->m_PackageSize;
    m_Destination = rFrom->m_Destination;
    m_Source      = rFrom->m_Source;
    m_pData.reset(new char[m_PackageSize]);
    memcpy(m_pData.get(), rFrom->m_pData.get(), m_PackageSize);
}

std::string CTCPGram::GetDescription()
{
    std::string ReturnString;
    int         Code = GetCode();
    switch (Code)
    {
        case TCPGRAM_CODE_INVALID:
            ReturnString = "Received INVALID";
            break;
        case TCPGRAM_CODE_DOUBLES:
            ReturnString = "Received DOUBLES";
            break;
        case TCPGRAM_CODE_COMMAND:
            ReturnString = "Received COMMAND";
            break;
        case TCPGRAM_CODE_STATUS:
            ReturnString = "Received STATUS";
            break;
        case TCPGRAM_CODE_STRING:
            ReturnString = "Received STRING";
            break;
        case TCPGRAM_CODE_INTERRUPT:
            ReturnString = "Received INTERRUPT";
            break;
        case TCPGRAM_CODE_WARNING:
            ReturnString = "Received WARNING";
            break;
        default:
            break;
    }
    return ReturnString;
}

unsigned char CTCPGram::GetCode()
{
    if (m_pData != nullptr && m_PackageSize > 0)
        return ::GetCode(m_pData.get());
    else
        return TCPGRAM_CODE_INVALID;
}

unsigned long CTCPGram::GetSize()
{
    if (m_pData != nullptr && m_PackageSize > 0)
        return ::GetSize(m_pData.get());
    else
        return 0;
}

std::unique_ptr<XMLElement> CTCPGram::GetXML()
{
    unsigned char DataType = GetCode();
    if (DataType != TCPGRAM_CODE_COMMAND && DataType != TCPGRAM_CODE_STATUS)
        return NULL;

    // get the oldest telegram and remove from the receive buffer
    const char   *pXMLString = (const char *)(&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD]));
    TiXmlDocument doc;
    if (strlen(pXMLString) != 0)
        if (!doc.Parse(pXMLString))
            THROW_ERROR(_T("TCP Error : the XML contents could not be decoded"));
    XMLElement *pXML = doc.FirstChildElement();
    RemoveElement(pXML); // remove from doc, otherwise doc's destructor will kill our XML
    std::unique_ptr<XMLElement> ReturnXML(pXML);
    return ReturnXML;
}

unsigned long CTCPGram::GetNumDoubles()
{
    unsigned long TeleGramsSize = GetSize();
    unsigned long NumDoubles    = (TeleGramsSize - TCPGRAM_HEADER_SIZE) / sizeof(double);
    return NumDoubles;
}

bool CTCPGram::GetDoubles(double *pVector, int iNumDoubles)
{
    unsigned char Code       = GetCode();
    unsigned long NumDoubles = GetNumDoubles();

    if (Code != TCPGRAM_CODE_DOUBLES)
        return false;

    if (NumDoubles != iNumDoubles)
    {
        std::string ErrorMessage;
        ErrorMessage.Format(_T("Error receiving data over TCP : expected %d numbers, received %d numbers"), iNumDoubles, NumDoubles);
        THROW_ERROR(ErrorMessage);
    }
    memcpy((void *)pVector, (&(m_pData.get()[TCPGRAM_INDEX_PAYLOAD])), sizeof(double) * NumDoubles);
    return true;
}

bool CTCPGram::GetString(std::string &rString)
{
    unsigned char Code          = GetCode();
    unsigned long TeleGramsSize = GetSize();
    int           StringLength  = (TeleGramsSize - TCPGRAM_HEADER_SIZE) / sizeof(TCHAR);

    if (Code != TCPGRAM_CODE_STRING)
        return false;

    rString.Empty();
    rString.Append(&m_pData.get()[TCPGRAM_INDEX_PAYLOAD]);
    return true;
}

void CTCPGram::Clear()
{
    m_pData.reset();
    m_PackageSize = 0;
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

bool CCommunicationInterface::GetReceivePackage(std::unique_ptr<CTCPGram> &ReturnTCPGram)
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

std::string CCommunicationInterface::GetError()
{
    CSingleLock Lock(&m_Lock, true);
    return m_ErrorString;
}

void CCommunicationInterface::SetError(LPCTSTR iErrorString)
{
    CSingleLock Lock(&m_Lock, true);
    m_bErrorOccurred = true;
    m_ErrorString    = iErrorString;
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

std::string CCommunicationInterface::GetHostName()
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

bool CCommunicationInterface::EditSettingsChanged()
{
#ifdef CTRACK_UI
    if (m_pPropertyPage.get())
    {
        return (m_CommunicationMode != static_cast<E_COMMUNICATION_Mode>(m_pPropertyPage->m_ComboDataType) || m_Port != m_pPropertyPage->m_Port ||
                m_bUDPBroadCast != m_pPropertyPage->m_bUDPBroadCast || m_HostName != m_pPropertyPage->m_HostName || m_PortUDP != m_pPropertyPage->m_PortUDP);
    }
#endif
    return false;
}

void CCommunicationInterface::EditSettings(CPropertySheet *pDialog)
{
#ifdef CTRACK_UI
    if (pDialog)
    {
        if (m_pPropertyPage.get() == nullptr)
            m_pPropertyPage.reset(new CDCommunicationPage);
        pDialog->AddPage(m_pPropertyPage.get());
        m_pPropertyPage->m_ComboDataType = static_cast<int>(m_CommunicationMode);
        m_pPropertyPage->m_Port          = m_Port;
        m_pPropertyPage->m_PortUDP       = m_PortUDP;
        m_pPropertyPage->m_bUDPBroadCast = m_bUDPBroadCast;
        m_pPropertyPage->m_HostName      = m_HostName;
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
        }
    }
#endif
}

//------------------------------------------------------------------------------------------------------------------
/*
Communication interface class
*/
//------------------------------------------------------------------------------------------------------------------

#define ATTRIB_TCP_MODE        _T("tcp_mode")
#define ATTRIB_PORT            _T("port")
#define ATTRIB_PORT_UDP        _T("port_udp")
#define ATTRIB_SEND_BLOCKING   _T("blocking")
#define ATTRIB_SEND_NAGLE      _T("nagle")
#define ATTRIB_SEND_TIME_OUT   _T("time_out")
#define ATTRIB_UDP_BROADCAST   _T("udp_broadcast")
#define ATTRIB_UDP_DESTINATION _T("udp_destination")

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

void CCommunicationObject::XML_ReadWrite(XMLElement *&pXML, bool Read)
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

void CCommunicationObject::Open(E_COMMUNICATION_Mode iTcpMode, int iPort, int iPortUDP, LPCTSTR iIpAddress)
{
    // start the communication thread
    if (IsOpen())
        Close();
    m_CommunicationMode = iTcpMode;
    m_Port              = iPort;
    m_PortUDP           = iPortUDP;
    m_HostName          = iIpAddress;
    Open();
}

CSocket *CCommunicationObject::SocketCreate(SOCKET iSocket, E_COMMUNICATION_Mode Mode, SOCKADDR_IN *ipSockAddress, unsigned short UDPBroadCastPort,
                                            bool UDPBroadcast, LPCTSTR UDPSendAddress)
{
    return new CSocket(iSocket, Mode, ipSockAddress, UDPBroadCastPort, UDPBroadcast, UDPSendAddress);
}

bool CCommunicationObject::IsOpen()
{
    return m_pCommunicationThread != nullptr;
}

void CCommunicationObject::Open()
{
    if (!m_pCommunicationThread)
    {
        // when a communication object wants to open the communication channels, CCommunicationManager checks if there is already a thread running on the same
        // port, if yes then the communication object is added to the list of communication objects belonging to that port, if no, then we create a new thread
        m_pCommunicationThread.reset(new CCommunicationThread);
        m_pCommunicationThread->CopyFrom(this);
        m_pCommunicationThread->CommunicationObjectAdd(*this);
        m_pCommunicationThread->StartThread();
    }
}

void CCommunicationObject::Close()
{
    if (m_pCommunicationThread)
    {
        m_pCommunicationThread->CommunicationObjectRemove(*this);
        if (m_pCommunicationThread->CommunicationObjectGetNum() == 0)
            m_pCommunicationThread.reset();
    }
}

int CCommunicationObject::GetNumConnections()
{
    if (m_pCommunicationThread)
        return m_pCommunicationThread->GetNumConnections();
    return 0;
}

void CCommunicationObject::PushSendPackage(std::unique_ptr<CTCPGram> &rTCPGram)
{
    if (m_pCommunicationThread)
        m_pCommunicationThread->PushSendPackage(rTCPGram);
}

//------------------------------------------------------------------------------------------------------------------
/*
CSocket class : mainly
*/
//------------------------------------------------------------------------------------------------------------------

// we use a fix defined buffer for reading operations

CSocket::CSocket(SOCKET iSocket, E_COMMUNICATION_Mode iCommunicationMode, SOCKADDR_IN *ipSockAddress, int UDBroadCastPort, bool iUDPBroadcast,
                 LPCTSTR iUDPSendToAddress)
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
        throw CExceptionSocket(__FILE__, __LINE__, _T("Failed to set the Nagle option"), false);
        // THROW_SOCKET_ERROR(_T("Failed to set the Nagle option"),false);
    }
}

void CSocket::SetNonBlocking(bool bNonBlocking)
{
    unsigned long argList[1];
    argList[0] = (bNonBlocking ? 0 : 1);
    if (ioctlsocket(m_Socket, FIONBIO, argList) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(_T("Failed to set non-blocking option"), false);
    }
}

void CSocket::SetReuseAddress(bool bEnableReuseAddress)
{
    int on = (bEnableReuseAddress ? 1 : 0);
    if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(_T("Failed to set the reuse address option"), false);
    }
}

void CSocket::SetBroadcast(bool bEnableBroadcast)
{
    int optval = (bEnableBroadcast ? 1 : 0);
    if (setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(_T("Failed to set the broadcast option"), false);
    }
}

int CSocket::GetMaxUDPMessageSize()
{
    int maxSize = 0;
    int SizeOf  = sizeof(int);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&maxSize, &SizeOf) == SOCKET_ERROR)
    {
        int LastError = WSAGetLastError();
        THROW_SOCKET_ERROR(_T("Failed to set the reuse address option"), false);
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
            THROW_SOCKET_ERROR(_T("Connection was closed"), true); // connection was closed
        else
            THROW_SOCKET_ERROR(_T("An error occurred on the TCP network"), false);
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
        {
            std::cout << "CSocket closed " << std::endl;
            throw gcnew CNetworkException(gcnew System::String(_T("Connection was closed")), true); // connection was closed
        }
        else
            THROW_SOCKET_ERROR(_T("An error occurred trying to read data from the network"), false);
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
        ReturnTCPGram->SetSource(m_Socket);
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
            int NumBytesWritten =
                send(m_Socket, &(rTCPGram->m_pData.get()[m_TotalBytesWritten]), MIN(RECEIVEBUFFERLENGTH, rTCPGram->m_PackageSize - m_TotalBytesWritten), 0);
            if (NumBytesWritten != SOCKET_ERROR)
            {
                m_TotalBytesWritten += NumBytesWritten;
                return (m_TotalBytesWritten == rTCPGram->m_PackageSize);
            }
            else // if the connection was closed, then remove the socket from the array
            {
                int LastError = WSAGetLastError();
                if (LastError == WSAECONNRESET || LastError == WSAENOTCONN || LastError == WSAECONNABORTED)
                    THROW_SOCKET_ERROR(_T("Connection was closed"), true); // connection was closed
                else
                    THROW_SOCKET_ERROR(_T("An error occurred trying to send data over the TCP network"), false);
            }
        };
        break;
        case UDP:
        {
            if (rTCPGram->m_PackageSize > m_MaxUDPMessageSize)
            {
                std::string ErrorMessage;
                ErrorMessage.Format(_T("Error sending data over UDP : the package size (%d) is bigger than allowed (%d)"), rTCPGram->m_PackageSize,
                                    m_MaxUDPMessageSize);
                THROW_ERROR(ErrorMessage);
            }
            int rVal = ::sendto(m_Socket, rTCPGram->m_pData.get(), rTCPGram->m_PackageSize, 0, (SOCKADDR *)&m_UDPBroadCastAddr, sizeof(m_UDPBroadCastAddr));
            if (rVal == SOCKET_ERROR)
            {
                int LastError = WSAGetLastError();
                THROW_SOCKET_ERROR(_T("An error occurred trying to send data over the UDP network"), false);
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
    m_Port              = STATEMANAGER_DEFAULT_TCP_PORT;
    m_HostName          = STATEMANAGER_DEFAULT_TCP_HOST;
    CSingleLock Lock(&m_Lock, true);
    m_IterCurrentSocket = m_arSockets.begin();
}

void CCommunicationThread::SetQuit(bool ibQuit)
{
    CSingleLock Lock(&m_Lock, true);
    m_bQuit = ibQuit;
}

bool CCommunicationThread::GetQuit()
{
    CSingleLock Lock(&m_Lock, true);
    return m_bQuit;
}

void CCommunicationThread::CommunicationObjectAdd(CCommunicationObject &rCommunicationObject)
{
    CSingleLock Lock(&m_Lock, true);
    m_setCommunicationObject.insert(&rCommunicationObject);
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
                                     bool bAddToNewComerList, bool UDPBroadcast, LPCTSTR UDPSendPort)
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
        assert(false);
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

void CCommunicationThread::SetError(LPCTSTR iErrorString)
{
    // copy error to all CCommunicationObjects
    CSingleLock Lock(&m_Lock, true);
    for (auto iter : m_setCommunicationObject)
        iter->SetError(iErrorString);
}

void CCommunicationThread::StartThread()
{
    assert(m_pExecuteThread == nullptr);
    if (!m_pExecuteThread)
    {
        m_bErrorOccurred                = false;
        m_pExecuteThread                = AfxBeginThread(CommunicationThread, (void *)this, THREAD_PRIORITY_NORMAL, CREATE_SUSPENDED);
        m_pExecuteThread->m_bAutoDelete = false;
        m_pExecuteThread->ResumeThread();
    }
}

void CCommunicationThread::EndThread()
{
    assert(m_pExecuteThread != nullptr);
    if (m_pExecuteThread)
    {
        SetQuit(true);
        int nResult = WaitForSingleObject(m_pExecuteThread->m_hThread, 5000);
        switch (nResult)
        {
            case WAIT_OBJECT_0:
                break;           // success
            case WAIT_ABANDONED: // nResult = 128
            case WAIT_TIMEOUT:   // nResult = 258
            case WAIT_FAILED:    // nResult = 0xFFFFFFFF
            {
                int LastError = GetLastError();
                assert(false);
            };
            break;
        }

        delete m_pExecuteThread;
        m_pExecuteThread = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------
/*
This is the main communication thread
*/
//------------------------------------------------------------------------------------------------------------------
UINT CommunicationThread(LPVOID lpParameter)
{
    CCommunicationThread *pTCPParameters = (CCommunicationThread *)lpParameter;
    ASSERT(pTCPParameters != nullptr);
    SOCKET               MainSocket = NULL; // for server mode : listen socket / client mode : client socket / udp : communication socket
    SOCKADDR_IN          sincontrol;
    E_COMMUNICATION_Mode CommunicationMode = pTCPParameters->GetCommunicationMode();
    WORD                 sockVersion;
    WSADATA              wsaData;

    try
    {
        pTCPParameters->SetQuit(false);

        //--------------------------------------------------------------------------------------------------------
        // Initialize socket library
        //--------------------------------------------------------------------------------------------------------
        sockVersion = MAKEWORD(2, 2);
        // start dll
        if (WSAStartup(sockVersion, &wsaData) != 0)
            THROW_ERROR(_T("Failed to initialize the socket library"));

        //--------------------------------------------------------------------------------------------------------
        // Resolve IP4 address (no need for IP6)
        //--------------------------------------------------------------------------------------------------------
        std::string    IP4;
        std::string    HostName      = pTCPParameters->GetHostName();
        unsigned short PortNumber    = pTCPParameters->GetPort();
        unsigned short PortNumberUDP = pTCPParameters->GetPortUDP();
        bool           bUDPBroadcast = pTCPParameters->GetUDPBroadcast();

        if (!ResolveIP4_Address(HostName, IP4))
        {
            std::string ErrorMessage;
            ErrorMessage.Format(_T("The host %s could not be resolved"), HostName);
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
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage;
                    ErrorMessage.Format(_T("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumber);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
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
                    std::string ErrorMessage;
                    ErrorMessage.Format(_T("The creation of the socket (host : %s port:%d) failed. Lasterror was %d."), HostName, PortNumber, LastError);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
                }

                // make it NON BLOCKING
                unsigned long argList[1];
                argList[0] = 1;
                int rVal   = ioctlsocket(MainSocket, FIONBIO, argList);
                if (rVal == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage;
                    ErrorMessage.Format(_T("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumber);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage;
                    ErrorMessage.Format(_T("The binding of the socket (host : %s port:%d) failed.Lasterror was %d.\nPossibly the port is already in use."),
                                        HostName, PortNumber, LastError);
                    DebugPrint(ErrorMessage);
                    AfxMessageBox(ErrorMessage, MB_ICONERROR);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
                }
                //
                // start listening
                if (::listen(MainSocket, 1 /*backlog amount permitted*/) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage;
                    ErrorMessage.Format(
                        _T("The listen command on the socket (host : %s port:%d) failed.Lasterror was %d.\nPossibly the port is already in use."), HostName,
                        PortNumber, LastError);
                    std::cout << ErrorMessage << std::endl;
                    AfxMessageBox(ErrorMessage, MB_ICONERROR);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
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
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage;
                    ErrorMessage.Format(_T("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumberUDP);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
                }

                // bind the socket
                if (::bind(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) == SOCKET_ERROR)
                {
                    int         LastError = WSAGetLastError();
                    std::string ErrorMessage;
                    ErrorMessage.Format(_T("The binding of the socket (host : %s port:%d) failed.Lasterror was %d.\nPossibly the port is already in use."),
                                        HostName, PortNumberUDP, LastError);
                    std::cout << ErrorMessage << std::endl;
                    AfxMessageBox(ErrorMessage, MB_ICONERROR);
                    THROW_SOCKET_ERROR(ErrorMessage, false);
                }
                pTCPParameters->SocketAdd(MainSocket, UDP, &sincontrol, PortNumber, false, bUDPBroadcast, HostName);

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
        DebugPrint("Starting thread for port : ", PortNumber);
        while (true)
        {
            if (pTCPParameters->GetQuit())
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
                            pTCPParameters->SocketAdd(ClientSocket, TCP_SERVER, &sincontrol, 0, true, false, _T(""));
                            DebugPrint("TCP server accepting client on port ", PortNumber);
                        }
                    }
                };
                break;
                case TCP_CLIENT:
                {
                    if (pTCPParameters->GetNumConnections() == 0)
                    {
                        if (MainSocket == INVALID_SOCKET)
                        {
                            MainSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                            if (MainSocket == INVALID_SOCKET)
                            {
                                int         LastError = WSAGetLastError();
                                std::string ErrorMessage;
                                ErrorMessage.Format(_T("The creation of the socket (host : %s port:%d) failed."), HostName, PortNumber);
                                THROW_SOCKET_ERROR(ErrorMessage, false);
                            }
                            ZeroMemory(&sincontrol, sizeof(sincontrol));
                            sincontrol.sin_family      = PF_INET;
                            sincontrol.sin_port        = htons(PortNumber);
                            sincontrol.sin_addr.s_addr = inet_addr(IP4);
                        }
                        if (connect(MainSocket, (LPSOCKADDR)&sincontrol, sizeof(sincontrol)) != SOCKET_ERROR)
                        {
                            DebugPrint("TCP client connected to ", HostName, " on port ", PortNumber);
                            pTCPParameters->SocketAdd(MainSocket, TCP_CLIENT, &sincontrol, 0, false, false, _T(""));
                        }
                        else
                        {
                            int SocketError = WSAGetLastError();
                            if ((SocketError != WSAECONNREFUSED) && (SocketError != WSAEWOULDBLOCK) && (SocketError != WSAEALREADY))
                                THROW_SOCKET_ERROR(_T("An error occurred trying to connect to the server"), false);
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
            bool                      bAvailable = pTCPParameters->GetSendPackage(TCPGram);
            while (bAvailable)
            {
                SOCKET   Destination          = TCPGram->GetDestination();
                bool     bAllSocketsCompleted = true;
                CSocket *pCurrentSocket       = pTCPParameters->SocketFirst();
                while (pCurrentSocket != nullptr)
                {
                    try
                    {
                        if (Destination == ALL_DESTINATIONS || Destination == pCurrentSocket->GetSocket())
                            if (!pCurrentSocket->WriteSend(TCPGram)) // returns false if sending of the telegram was not completed yet
                                bAllSocketsCompleted = false;
                        pCurrentSocket = pTCPParameters->SocketNext();
                    }
                    catch (CNetworkException ^ e) // disconnected, other exceptions are handled by outer routines
                    {
                        if (e->m_bDisconnected)
                        {
                            pCurrentSocket = pTCPParameters->SocketDeleteCurrent();
                            if (CommunicationMode == TCP_CLIENT)
                            {
                                DebugPrint("TCP client disconnected from ", HostName, " on port ", PortNumber);
                                MainSocket = INVALID_SOCKET;
                            }
                        }
                        else
                            throw e;
                    }
                    catch (System::Exception ^ e)
                    {
                        std::cout << "Hey" << std::endl;
                    }
                    catch (...)
                    {
                        std::cout << "Hey" << std::endl;
                    }
                }
                if (bAllSocketsCompleted) // get the next package
                {
                    bAvailable = pTCPParameters->GetSendPackage(TCPGram);
                    pTCPParameters->SocketResetSend();
                }
            }

            //--------------------------------------------------------------------------------------------------------
            // Receiving data
            //--------------------------------------------------------------------------------------------------------
            CSocket *pCurrentSocket = pTCPParameters->SocketFirst();
            while (pCurrentSocket != nullptr)
            {
                try
                {
                    pCurrentSocket->ReadFeedBuffer();
                    std::unique_ptr<CTCPGram> TCPGram;
                    bool                      bAvailable = pCurrentSocket->ReadExtractTelegram(TCPGram);
                    while (bAvailable)
                    {
                        pTCPParameters->PushReceivePackage(TCPGram);
                        DebugPrint("TCP received ", TCPGram->GetDescription());
                        bAvailable = pCurrentSocket->ReadExtractTelegram(TCPGram);
                    };
                    pCurrentSocket = pTCPParameters->SocketNext();
                }
                catch (CNetworkException ^ e) // disconnected, other exceptions are handled by outer routines
                {
                    if (e->m_bDisconnected)
                    {
                        pCurrentSocket = pTCPParameters->SocketDeleteCurrent();
                        if (CommunicationMode == TCP_CLIENT)
                        {
                            DebugPrint("TCP client disconnected from ", HostName, " on port ", PortNumber);
                            MainSocket = INVALID_SOCKET;
                        }
                    }
                    else
                        throw e;
                }
            }
        }
    }
    catch (System::Exception ^ e)
    {
        std::string Message(e->Message);
        DebugPrint(_T("Exception occurred : "), Message);
    }
    catch (CException *e)
    {
        TCHAR ErrorMessage[2001];
        e->GetErrorMessage(ErrorMessage, 2000);
        pTCPParameters->SetError(ErrorMessage);
        e->Delete();
    }

    //
    // close our sockets
    pTCPParameters->SocketDeleteAll();

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
    return 0;
}
