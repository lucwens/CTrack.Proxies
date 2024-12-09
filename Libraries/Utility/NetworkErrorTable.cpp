
#include "NetworkErrorTable.h"

std::map<int, WSA_ERROR_TABLE> NetworkErrorMap = {
    {WSAEINTR, {"WSAEINTR", "Interrupted function call.", "A blocking operation was interrupted by a call to WSACancelBlockingCall."}},
    {WSAEACCES,
     {"WSAEACCES", "Permission denied.",
      "An attempt was made to access a socket in a way forbidden by its access permissions. An example is using a broadcast address for sendto without "
      "broadcast permission being set using setsockopt (SO_BROADCAST). \r\n\r\nAnother possible reason for the WSAEACCES error is that when the bind function "
      "is called (on Windows NT 4 SP4 or later), another application, service, or kernel mode driver is bound to the same address with exclusive access. Such "
      "exclusive access is a new feature of Windows NT 4 SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option."}},
    {WSAEFAULT,
     {"WSAEFAULT", "Bad address.",
      "The system detected an invalid pointer address in attempting to use a pointer argument of a call. This error occurs if an application passes an "
      "invalid pointer value, or if the length of the buffer is too small. For instance, if the length of an argument, which is a sockaddr structure, is "
      "smaller than the sizeof(sockaddr)."}},
    {WSAEINVAL,
     {"WSAEINVA", "Invalid argument.",
      "Some invalid argument was supplied (for example, specifying an invalid level to the setsockopt function). In some instances, it also refers to the "
      "current state of the socket - for instance, calling accept on a socket that is not listening."}},
    {WSAEMFILE,
     {"WSAEMFILE", "Too many open files.",
      "Too many open sockets. Each implementation may have a maximum number of socket handles available, either globally, per process, or per thread."}},
    {WSAEWOULDBLOCK,
     {"WSAEWOULDBLOCK", "Resource temporarily unavailable.",
      "This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read "
      "from the socket. It is a nonfatal error, and the operation should be retried later. It is normal for WSAEWOULDBLOCK to be reported as the result from "
      "calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established."}},
    {WSAEINPROGRESS,
     {"WSAEINPROGRESS", "Operation now in progress.",
      "A blocking operation is currently executing. Windows Sockets only allows a single blocking operation per- task or thread to be outstanding, and if any "
      "other function call is made (whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error."}},
    {WSAEALREADY,
     {"WSAEALREADY", "Operation already in progress.",
      "An operation was attempted on a nonblocking socket with an operation already in progress - that is, calling connect a second time on a nonblocking "
      "socket that is already connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) that has already been canceled or completed."}},
    {WSAENOTSOCK,
     {"WSAENOTSOCK", "Socket operation on nonsocket.",
      "An operation was attempted on something that is not a socket. Either the socket handle parameter did not reference a valid socket, or for select, a "
      "member of an fd_set was not valid."}},
    {WSAEDESTADDRREQ,
     {"WSAEDESTADDRREQ", "Destination address required.",
      "A required address was omitted from an operation on a socket. For example, this error is returned if sendto is called with the remote address of "
      "ADDR_ANY."}},
    {WSAEMSGSIZE,
     {"WSAEMSGSIZE", "Message too long.",
      "A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram "
      "was smaller than the datagram itself."}},
    {WSAEPROTOTYPE,
     {"WSAEPROTOTYPE", "Protocol wrong type for socket.",
      "A protocol was specified in the socket function call that does not support the semantics of the socket type requested. For example, the ARPA Internet "
      "UDP protocol cannot be specified with a socket type of SOCK_STREAM."}},
    {WSAENOPROTOOPT,
     {"WSAENOPROTOOPT", "Bad protocol option.", "An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call."}},
    {WSAEPROTONOSUPPORT,
     {"WSAEPROTONOSUPPORT", "Protocol not supported.",
      "The requested protocol has not been configured into the system, or no implementation for it exists. For example, a socket call requests a SOCK_DGRAM "
      "socket, but specifies a stream protocol."}},
    {WSAESOCKTNOSUPPORT,
     {"WSAESOCKTNOSUPPORT", "Socket type not supported.",
      "The support for the specified socket type does not exist in this address family. For example, the optional type SOCK_RAW might be selected in a socket "
      "call, and the implementation does not support SOCK_RAW sockets at all."}},
    {WSAEOPNOTSUPP,
     {"WSAEOPNOTSUPP", "Operation not supported.",
      "The attempted operation is not supported for the type of object referenced. Usually this occurs when a socket descriptor to a socket that cannot "
      "support this operation is trying to accept a connection on a datagram socket."}},
    {WSAEPFNOSUPPORT,
     {"WSAEPFNOSUPPORT", "Protocol family not supported.",
      "The protocol family has not been configured into the system or no implementation for it exists. This message has a slightly different meaning from "
      "WSAEAFNOSUPPORT. However, it is interchangeable in most cases, and all Windows Sockets functions that return one of these messages also specify "
      "WSAEAFNOSUPPORT."}},
    {WSAEAFNOSUPPORT,
     {"WSAEAFNOSUPPORT", "Address family not supported by protocol family.",
      "An address incompatible with the requested protocol was used. All sockets are created with an associated address family (that is, AF_INET for Internet "
      "Protocols) and a generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect protocol is explicitly requested in the socket "
      "call, or if an address of the wrong family is used for a socket, for example, in sendto."}},
    {WSAEADDRINUSE,
     {"WSAEADDRINUSE", "Address already in use.",
      "Typically, only one usage of each socket address (protocol/IP address/port) is permitted. This error occurs if an application attempts to bind a "
      "socket to an IP address/port that has already been used for an existing socket, or a socket that was not closed properly, or one that is still in the "
      "process of closing. For server applications that need to bind multiple sockets to the same port number, consider using setsockopt (SO_REUSEADDR). "
      "Client applications usually need not call bind at all - connect chooses an unused port automatically. When bind is called with a wildcard address "
      "(involving ADDR_ANY), a WSAEADDRINUSE error could be delayed until the specific address is committed. This could happen with a call to another "
      "function later, including connect, listen, WSAConnect, or WSAJoinLeaf."}},
    {WSAEADDRNOTAVAIL,
     {"WSAEADDRNOTAVAI", "Cannot assign requested address.",
      "The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local "
      "computer. This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote "
      "computer (for example, address or port 0)."}},
    {WSAENETDOWN,
     {"WSAENETDOWN", "Network is down.",
      "A socket operation encountered a dead network. This could indicate a serious failure of the network system (that is, the protocol stack that the "
      "Windows Sockets DLL runs over), the network interface, or the local network itself."}},
    {WSAENETUNREACH,
     {"WSAENETUNREACH", "Network is unreachable.",
      "A socket operation was attempted to an unreachable network. This usually means the local software knows no route to reach the remote host."}},
    {WSAENETRESET,
     {"WSAENETRESET", "Network dropped connection on reset.",
      "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress. It can also be returned by "
      "setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed."}},
    {WSAECONNABORTED,
     {"WSAECONNABORTED", "Software caused connection abort.",
      "An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error."}},
    {WSAECONNRESET,
     {"WSAECONNRESET", "Connection reset by peer.",
      "An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly stopped, "
      "the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close (see setsockopt for more information on "
      "the SO_LINGER option on the remote socket). This error may also result if a connection was broken due to keep-alive activity detecting a failure while "
      "one or more operations are in progress. Operations that were in progress fail with WSAENETRESET. Subsequent operations fail with WSAECONNRESET."}},
    {WSAENOBUFS,
     {"WSAENOBUFS", "No buffer space available.",
      "An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full."}},
    {WSAEISCONN,
     {"WSAEISCONN", "Socket is already connected.",
      "A connect request was made on an already-connected socket. Some implementations also return this error if sendto is called on a connected SOCK_DGRAM "
      "socket (for SOCK_STREAM sockets, the to parameter in sendto is ignored) although other implementations treat this as a legal occurrence."}},
    {WSAENOTCONN,
     {"WSAENOTCONN", "Socket is not connected.",
      "A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address "
      "was supplied. Any other type of operation might also return this error - for example, setsockopt setting SO_KEEPALIVE if the connection has been "
      "reset."}},
    {WSAESHUTDOWN,
     {"WSAESHUTDOWN", "Cannot send after socket shutdown.",
      "A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call. By "
      "calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued."}},
    {WSAETIMEDOUT,
     {"WSAETIMEDOUT", "Connection timed out.",
      "A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because "
      "the connected host has failed to respond."}},
    {WSAECONNREFUSED,
     {"WSAECONNREFUSED", "Connection refused.",
      "No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is inactive "
      "on the foreign host - that is, one with no server application running."}},
    {WSAEHOSTDOWN,
     {"WSAEHOSTDOWN", "Host is down.",
      "A socket operation failed because the destination host is down. A socket operation encountered a dead host. Networking activity on the local host has "
      "not been initiated. These conditions are more likely to be indicated by the error WSAETIMEDOUT."}},
    {WSAEHOSTUNREACH, {"WSAEHOSTUNREACH", "No route to host.", "A socket operation was attempted to an unreachable host. See WSAENETUNREACH."}},
    {WSAEPROCLIM,
     {"WSAEPROCLIM", "Too many processes.",
      "A Windows Sockets implementation may have a limit on the number of applications that can use it simultaneously. WSAStartup may fail with this error if "
      "the limit has been reached."}},
    {WSASYSNOTREADY,
     {"WSASYSNOTREADY", "Network subsystem is unavailable.",
      "This error is returned by WSAStartup if the Windows Sockets implementation cannot function at this time because the underlying system it uses to "
      "provide network services is currently unavailable. Users should check: \r\n1. That the appropriate Windows Sockets DLL file is in the current path; "
      "\r\n2. That they are not trying to use more than one Windows Sockets implementation simultaneously. If there is more than one Winsock DLL on your "
      "system, be sure the first one in the path is appropriate for the network subsystem currently loaded; \r\n3. The Windows Sockets implementation "
      "documentation to be sure all necessary components are currently installed and configured correctly."}},
    {WSAVERNOTSUPPORTED,
     {"WSAVERNOTSUPPORTED", "Winsock.dll version out of range.",
      "The current Windows Sockets implementation does not support the Windows Sockets specification version requested by the application. Check that no old "
      "Windows Sockets DLL files are being accessed."}},
    {WSANOTINITIALISED,
     {"WSANOTINITIALISED", "Successful WSAStartup not yet performed.",
      "Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not "
      "own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times."}},
    {WSAEDISCON,
     {"WSAEDISCON", "Graceful shutdown in progress.",
      "Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a graceful shutdown sequence."}},
    {WSATYPE_NOT_FOUND, {"WSATYPE_NOT_FOUND", "Class type not found.", "The specified class was not found."}},
    {WSAHOST_NOT_FOUND,
     {"WSAHOST_NOT_FOUND", "Host not found.",
      "No such host is known. The name is not an official host name or alias, or it cannot be found in the database(s) being queried. This error may also be "
      "returned for protocol and service queries, and means that the specified name could not be found in the relevant database."}},
    {WSATRY_AGAIN,
     {"WSATRY_AGAIN", "Nonauthoritative host not found.",
      "This is usually a temporary error during host name resolution and means that the local server did not receive a response from an authoritative server. "
      "A retry at some time later may be successful."}},
    {WSANO_RECOVERY,
     {"WSANO_RECOVERY", "This is a nonrecoverable error.",
      "This indicates that some sort of nonrecoverable error occurred during a database lookup. This may be because the database files (for example, "
      "BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned by the server with a severe error."}},
    {WSANO_DATA,
     {"WSANO_DATA", "Valid name, no data record of requested type.",
      "The requested name is valid and was found in the database, but it does not have the correct associated data being resolved for. The usual example for "
      "this is a host name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) which uses the DNS (Domain Name Server). An MX "
      "record is returned but no A record - indicating the host itself exists, but is not directly reachable."}},
    {WSA_INVALID_HANDLE,
     {"WSA_INVALID_HANDLE", "Specified event object handle is invalid.",
      "An application attempts to use an event object, but the specified handle is not valid."}},
    {WSA_INVALID_PARAMETER,
     {"WSA_INVALID_PARAMETER", "One or more parameters are invalid.",
      "An application used a Windows Sockets function which directly maps to a Windows  function. The Windows function is indicating a problem with one or "
      "more parameters."}},
    {WSA_IO_INCOMPLETE,
     {"WSA_IO_INCOMPLETE", "Overlapped I/O event object not in signaled state.",
      "The application has tried to determine the status of an overlapped operation which is not yet completed. Applications that use WSAGetOverlappedResult "
      "(with the fWait flag set to FALSE) in a polling mode to determine when an overlapped operation has completed, get this error code until the operation "
      "is complete."}},
    {WSA_IO_PENDING,
     {"WSA_IO_PENDING", "Overlapped operations will complete later.",
      "The application has initiated an overlapped operation that cannot be completed immediately. A completion indication will be given later when the "
      "operation has been completed."}},
    {WSA_NOT_ENOUGH_MEMORY,
     {"WSA_NOT_ENOUGH_MEMORY", "Insufficient memory available.",
      "An application used a Windows Sockets function that directly maps to a Windows function. The Windows function is indicating a lack of required memory "
      "resources."}},
    {WSA_OPERATION_ABORTED,
     {"WSA_OPERATION_ABORTED", "Overlapped operation aborted.",
      "An overlapped operation was canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl."}},
    {WSAEINVALIDPROCTABLE,
     {"WSAEINVALIDPROCTABLE", "Invalid procedure table from service provider.",
      "A service provider returned a bogus procedure table to Ws2_32.dll. (This is usually caused by one or more of the function pointers being null.)"}},
    {WSAEINVALIDPROVIDER, {"WSAEINVALIDPROVIDER", "Invalid service provider version number.", "A service provider returned a version number other than 2.0."}},
    {WSAEPROVIDERFAILEDINIT,
     {"WSAEPROVIDERFAILEDINIT", "Unable to initialize a service provider.",
      "Either a service provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup NSPStartup function failed."}},
    {WSASYSCALLFAILURE, {"WSASYSCALLFAILURE", "System call failure.", "System call failure."}}};
