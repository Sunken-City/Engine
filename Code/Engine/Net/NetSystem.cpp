#include "Engine/Net/NetSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/Logging.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"


//Code based off of code written by Professor Christopher Forseth

NetSystem* NetSystem::instance = nullptr;

//-----------------------------------------------------------------------------------
NetSystem::NetSystem()
{
    WSADATA wsa_data;

    //Startup Winsock version 2.2
    int error = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    if (error != 0) 
    {
        ERROR_AND_DIE(Stringf("Failed to initialize WinSock. Error[%u]\n", error));
    }
}

//-----------------------------------------------------------------------------------
NetSystem::~NetSystem()
{
    WSACleanup();
}

//-----------------------------------------------------------------------------------
//Get Local Host Name (stored in a global buffer - NOT THREAD SAFE)
const char* NetSystem::GetLocalHostName()
{
    static char buffer[256];
    if (gethostname(buffer, 256) == 0) 
    {
        return buffer;
    }
    else 
    {
        int error = WSAGetLastError();
        ERROR_RECOVERABLE(Stringf("Failed to get the local host name. Error[%u]\n", error));
        return "localhost";
    }
}

//-----------------------------------------------------------------------------------
// Get All Addresses that match our criteria
addrinfo* NetSystem::AllocAddressesForHost(char const* hostName, // host, like google.com
                                           char const* portNumber, // service, usually the port number as a string
                                           int connectionFamily,      // Connection Family, AF_INET (IPv4) for this assignment
                                           int socketType,    // Socket Type, SOCK_STREAM or SOCK_DGRAM (TCP or UDP) for this class
                                           int flags)  // Search flag hints, we use this for AI_PASSIVE (bindable addresses)
{
    //Also, family of AF_UNSPEC will return all address that support the sock type (so both IPv4 and IPv6 adddress).

    //Define the hints - this is what it will use for determining what addresses to return
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = connectionFamily;
    hints.ai_socktype = socketType;
    hints.ai_flags = flags;

    //This will allocate all addresses into a single linked list with the head put into result.
    addrinfo* result = nullptr;
    int status = getaddrinfo(hostName, portNumber, &hints, &result);
    if (status != 0) 
    {
        LogPrintf(LogLevel::WARNING, "Network: Failed to find addresses for [%s:%s]. Error[%s]", hostName, portNumber, gai_strerror(status));
        return nullptr;
    }

    return result;
}

//-----------------------------------------------------------------------------------
//Since we allocate the addresses, we must free them.
//Takes the addrinfo written to by getaddrinfo(...)
void NetSystem::FreeAddresses(addrinfo* addresses)
{
    if (nullptr != addresses) 
    {
        freeaddrinfo(addresses);
    }
}

//-----------------------------------------------------------------------------------
sockaddr* NetSystem::GetLocalHostAddressUDP(const char* portNumber)
{
#pragma todo("I'm pretty sure this leaks memory, clean this up")
    addrinfo* infoList = AllocAddressesForHost(GetLocalHostName(), // an address for this machine
        portNumber, //service, which for TCP/IP is the port as a string (ex: "80")
        AF_INET, //We're doing IPv4 in class
        SOCK_DGRAM, //UDP
        AI_PASSIVE);  //And something we can bind (and therefore listen on)
    if (infoList != nullptr)
    {
        return infoList->ai_addr;
    }
    else
    {
        return nullptr;
    }
}

//-----------------------------------------------------------------------------------
bool NetSystem::SockaddrCompare(const sockaddr_in& first, const sockaddr_in& second)
{
#pragma todo("Move this into a sockaddr wrapper class")
    bool addressesMatch = ntohl(first.sin_addr.s_addr) == ntohl(second.sin_addr.s_addr);
    bool portsMatch = ntohs(first.sin_port) == ntohs(second.sin_port);
    return addressesMatch && portsMatch;
}

//-----------------------------------------------------------------------------------
// Binding a TCP Socket for Listening Purposes
SOCKET NetSystem::CreateListenSocket(const char* hostName, 
                                     const char* portNumber, // who we're trying to connect to
                                     sockaddr_in* outAddress) // address we actually connected to.
{
    // First, try to get network addresses for this
    addrinfo *infoList = AllocAddressesForHost(hostName, // an address for this machine
        portNumber, //service, which for TCP/IP is the port as a string (ex: "80")
        AF_INET, //We're doing IPv4 in class
        SOCK_STREAM, //TCP for now
        AI_PASSIVE);  //And something we can bind (and therefore listen on)


    if (infoList == nullptr) 
    {
        //No addresses match - FAIL
        return false;
    }

    //Alright, try to create a SOCKET from this address info
    SOCKET mySocket = INVALID_SOCKET;
    addrinfo* iter = infoList;
    while ((iter != nullptr) && (mySocket == INVALID_SOCKET)) 
    {
        //First, create a socket for this address.
        //family, socktype, and protocol are provided by the addrinfo
        //if you wanted to be manual, for an TCP/IPv4 socket you'd use
        //AF_INET, SOCK_STREAM, IPPROTO_TCP
        mySocket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
        if (mySocket != INVALID_SOCKET) 
        {
            //Okay, we were able to create it,
            //Now try to bind it (associates the address (ex: 192.168.1.52:4325) to this 
            //socket so it will receive information for it.
            int result = bind(mySocket, iter->ai_addr, (int)(iter->ai_addrlen));
            if (SOCKET_ERROR != result) {

                // Set it to non-block - since we'll be working with this on our main thread
                u_long non_blocking = 1;
                ioctlsocket(mySocket, FIONBIO, &non_blocking);

                // Set it to listen - this will allow people to connect to us
                result = listen(mySocket, 2);
                ASSERT_OR_DIE(result != SOCKET_ERROR, "Error occurred while creating a listen socket."); // sanity check

                // Save off the address if available.
                ASSERT_OR_DIE(iter->ai_addrlen == sizeof(sockaddr_in), "Error occurred while creating a listen socket.");
                if (nullptr != outAddress) 
                {
                    memcpy(outAddress, iter->ai_addr, iter->ai_addrlen);
                }
            }
            else 
            {
                // Cleanup on Fail.
                closesocket(mySocket);
                mySocket = INVALID_SOCKET;
            }
        }
        iter = iter->ai_next;
    }

    //If we allocted, we must free eventually
    FreeAddresses(infoList);

    //Return the socket we created.
    return mySocket;
}

//-----------------------------------------------------------------------------------
//Accepting a Connection on a Listening Socket
//(Called on the host - this will Okay a socket trying to connect with ::connect in CreateListenSocket).
SOCKET NetSystem::AcceptConnection(SOCKET hostSocket, sockaddr_in* outTheirAddress)
{
    sockaddr_storage theirAddress;
    int theirAddressLength = sizeof(theirAddress);

    SOCKET TheirSocket = ::accept(hostSocket, (sockaddr*)&theirAddress, &theirAddressLength);
    if (TheirSocket != INVALID_SOCKET) 
    {
        //If you want to support IPv6, this is no longer a valid check
        if (outTheirAddress != nullptr) 
        {
            ASSERT_OR_DIE(theirAddressLength == sizeof(sockaddr_in), "Address length didn't match expected value. (Are you trying to use IPV6?)");
            memcpy(outTheirAddress, &theirAddress, theirAddressLength);
        }

        return TheirSocket;
    }
    else 
    {
        // if we fail to accept, it might be we lost
        // connection - you can check the same we we'll do it
        // for send and recv below, and potentially return 
        // that error code somehow (if you move this code into a method
        // you could disonnect directly)
        
        int err = WSAGetLastError();
        if (SocketErrorShouldDisconnect(err)) 
        {
            //disconnect();
            ERROR_AND_DIE("Failed to accept the connection");
        }
    }

    return INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------
// Similar to Hosting - with a few changes internally
// Comments are only around places that change.
SOCKET NetSystem::JoinSocket(const char* address, const char* service, sockaddr_in* outAddress)
{
    // First, we don't look for AI_PASSIVE connections - we don't bind this one
    // We'll be sending to it.
    addrinfo* infoList = AllocAddressesForHost(address, service, AF_INET, SOCK_STREAM, 0);
    if (infoList == nullptr) {
        return false;
    }

    SOCKET mySocket = INVALID_SOCKET;
    addrinfo* iter = infoList;
    while ((iter != nullptr) && (mySocket == INVALID_SOCKET)) 
    {
        mySocket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
        if (mySocket != INVALID_SOCKET) 
        {
            //Instead of binding - we call connect, which will do the TCP/IP handshake.
            //Be aware this can block and cause a frame hickup, which is fine for now.
            int result = ::connect(mySocket, iter->ai_addr, (int)(iter->ai_addrlen));
            if (SOCKET_ERROR != result) 
            {
                u_long non_blocking = 1;
                ioctlsocket(mySocket, FIONBIO, &non_blocking);

                //We do not listen on on this socket - we are not
                //accepting new connections.
                ASSERT_OR_DIE(iter->ai_addrlen == sizeof(sockaddr_in), "Error while joining socket with address length. (IPV6 problem?)");
                if (nullptr != outAddress) 
                {
                    memcpy(outAddress, iter->ai_addr, iter->ai_addrlen);
                }
            }
            else 
            {
                int error = WSAGetLastError();
                ERROR_RECOVERABLE(Stringf("Failed to Join. Error[%u]\n", error));
                closesocket(mySocket);
                mySocket = INVALID_SOCKET;
            }
        }
        iter = iter->ai_next;
    }

    FreeAddresses(infoList);

    return mySocket;
}

//-----------------------------------------------------------------------------------
void NetSystem::CloseSocket(SOCKET sock)
{
    closesocket(sock);
}

//-----------------------------------------------------------------------------------
size_t NetSystem::SendOnSocket(bool& outShouldDisconnect, SOCKET mySocket, const void* data, const size_t dataSize)
{
    outShouldDisconnect = false;
    if (mySocket != INVALID_SOCKET) 
    {
        //send will return the amount of data actually sent. It SHOULD match, or be an error.  
        int size = ::send(mySocket, (char const*)data, (int)dataSize, 0);
        if (size < 0) 
        {
            int32_t error = WSAGetLastError();
            if (SocketErrorShouldDisconnect(error)) 
            {
                //If the error is critical - disconnect this socket
                outShouldDisconnect = true;
            }
        }
        else 
        {
            ASSERT_OR_DIE((size_t)size == dataSize, "The size of the data sent wasn't the same as the size we attempted to send");
        }

        return (size_t)size;
    }
    else
    {
        ERROR_RECOVERABLE("Attempted to send on an invalid socket.");
    }
    return 0U;
}

//-----------------------------------------------------------------------------------
size_t NetSystem::RecieveFromSocket(bool& outShouldDisconnect, SOCKET mySocket, void* buffer, const size_t bufferSize)
{
    outShouldDisconnect = false;
    if (mySocket != INVALID_SOCKET) 
    {
        // recv will return amount of data read, should always be <= buffer_size
        // Also, if you send, say, 3 KB with send, recv may actually
        // end up returning multiple times (say, 1KB, 512B, and 1.5KB) because 
        // the message got broken up - so be sure you application watches for it
        int size = ::recv(mySocket, (char*)buffer, bufferSize, 0);
        if (size < 0) 
        {
            int32_t error = WSAGetLastError();
            if (SocketErrorShouldDisconnect(error)) 
            {
                outShouldDisconnect = true;
            }
            return 0U;
        }
        else 
        {
            return (size_t)size;
        }
    }
    else 
    {
        return 0U;
    }
}

//-----------------------------------------------------------------------------------
//Converting a sockaddr_in to a String
//Again, doing this in a global buffer so it is NOT THREAD SAFE
const char* NetSystem::SockAddrToString(const sockaddr* address)
{
    static char buffer[256];

    if (!address)
    {
        return "Invalid Sockaddr";
    }

    // Hard coding this for sockaddr_in for brevity
    // You can make this work for IPv6 as well
    sockaddr_in *addr_in = (sockaddr_in*)address;

    // inet_ntop converts an address type to a human readable string,
    // ie 0x7f000001 => "127.0.0.1"
    // GetInAddr (defined below) gets the pointer to the address part of the sockaddr
    char hostname[256];
    inet_ntop(addr_in->sin_family, GetInAddr(address), hostname, 256);

    // Combine the above with the port.  
    // Port is stored in network order, so convert it to host order
    // using ntohs (Network TO Host Short)
    sprintf_s(buffer, 256, "%s:%u", hostname, ntohs(addr_in->sin_port));

    // buffer is static - so will not go out of scope, but that means this is not thread safe.
    return buffer;
}

//-----------------------------------------------------------------------------------
bool NetSystem::StringToSockAddrIPv4(sockaddr_in& outSockAddr, const char* string)
{
#pragma todo("Does this actually work?")
    unsigned long address = inet_addr(string);
    if (address == INADDR_NONE) 
    {
        LogPrintf(LogLevel::SEVERE, "StringToSockAddrIPv4 failed and returned INADDR_NONE.\n");
        return false;
    }
    if (address == INADDR_ANY) 
    {
        LogPrintf(LogLevel::SEVERE, "StringToSockAddrIPv4 failed and returned INADDR_ANY.\n");
        return false;
    }
    outSockAddr.sin_addr.S_un.S_addr = address;
    return true;
}

//-----------------------------------------------------------------------------------
sockaddr_in NetSystem::StringToSockAddrIPv4(const char* ip, const uint16_t port)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_addr.S_un.S_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    return addr;
}


//-----------------------------------------------------------------------------------
//Get address part of a sockaddr, IPv4 or IPv6:
void* NetSystem::GetInAddr(const sockaddr* socketAddress)
{
    if (socketAddress->sa_family == AF_INET) 
    {
        return &(((sockaddr_in*)socketAddress)->sin_addr);
    }
    else 
    {
        return &(((sockaddr_in6*)socketAddress)->sin6_addr);
    }
}

//-----------------------------------------------------------------------------------
// Ignoring Non-Critical Errors
// These errors are non-fatal and are more or less ignorable.
bool NetSystem::SocketErrorShouldDisconnect(const int32_t error)
{
    switch (error) 
    {
    case WSAEWOULDBLOCK: // nothing to do - would've blocked if set to blocking
    case WSAEMSGSIZE:    // UDP message too large - ignore that packet.
    case WSAECONNRESET:  // Other side reset their connection.
        return false;

    default:
        return true;
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(getlocalhostname)
{
    UNUSED(args);
    Console::instance->PrintLine(NetSystem::instance->GetLocalHostName(), RGBA::GBDARKGREEN);
}