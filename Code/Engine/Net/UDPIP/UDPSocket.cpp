#include "Engine/Net/UDPIP/UDPSocket.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "../../Input/Logging.hpp"

//-----------------------------------------------------------------------------------
SOCKET UDPSocket::CreateUDPSocket(char const* address,
    char const* portNumber, // who we're trying to connect to
    sockaddr_in* out_addr) // address we actually connected to.
{
    const int MAX_RETRIES = 8;
    int currNumRetries = 0;

    while (m_socket == INVALID_SOCKET && currNumRetries < MAX_RETRIES)
    {
        //Set up the portNumber
        size_t bufferSize = strlen(portNumber) + 1;
        char* portStr = new char[bufferSize];
        unsigned int port = atoi(portNumber) + currNumRetries;
        _itoa_s(port, portStr, bufferSize, 10);

        // First, try to get network addresses for this
        addrinfo *info_list = NetSystem::AllocAddressesForHost(address, // an address for this machine
            portStr, // service, which for TCP/IP is the port as a string (ex: "80")
            AF_INET, // We're doing IPv4 in class
            SOCK_DGRAM, // UDP for now
            AI_PASSIVE);  // And something we can bind (and therefore listen on)

        if (info_list == nullptr) 
        {
            // no addresses match - FAIL
            return false;
        }

        // Alright, try to create a SOCKET from this addr info
        SOCKET my_socket = INVALID_SOCKET;
        addrinfo *iter = info_list;
        while ((iter != nullptr) && (my_socket == INVALID_SOCKET)) 
        {
            // First, create a socket for this address.
            // family, socktype, and protocol are provided by the addrinfo
            // if you wanted to be manual, for an TCP/IPv4 socket you'd use
            // AF_INET, SOCK_DGRAM, IPPROTO_UDP
            my_socket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
            if (my_socket != INVALID_SOCKET) 
            {
                // Okay, we were able to create it,
                // Now try to bind it (associates the address (ex: 192.168.1.52:4325) to this 
                // socket so it will receive information for it.
    
                int result = bind(my_socket, iter->ai_addr, (int)(iter->ai_addrlen));
                if (SOCKET_ERROR != result)
                {
                    // Set it to non-block - since we'll be working with this on our main thread
                    u_long non_blocking = 1;
                    ioctlsocket(my_socket, FIONBIO, &non_blocking);

                    // Save off the address if available.
                    ASSERT_OR_DIE(iter->ai_addrlen == sizeof(sockaddr_in), "Address length doesn't match.");
                    if (nullptr != out_addr)
                    {
                        memcpy(out_addr, iter->ai_addr, iter->ai_addrlen);
                    }
                }
                else
                {
                    // Cleanup on Fail.
                    closesocket(my_socket);
                    my_socket = INVALID_SOCKET;
                }

            }
            iter = iter->ai_next;
        }
        ++currNumRetries;
        delete portStr;

        // If we allocted, we must free eventually
        NetSystem::FreeAddresses(info_list);

        // Return the socket we created.
        if (my_socket != INVALID_SOCKET)
        {
            return my_socket;
        }
    }
    if (currNumRetries == MAX_RETRIES)
    {
        LogPrintf(LogLevel::WARNING, "Was unable to bind to a port after %i retries", MAX_RETRIES);
    }
    return INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------
size_t UDPSocket::SendTo(const sockaddr_in& toAddress, void const* data, const size_t dataSize)
{
    if (m_socket != INVALID_SOCKET) 
    {
        // send will return the amount of data actually sent.
        // It SHOULD match, or be an error.  
        int size = ::sendto(m_socket,
            (char const*)data,      // payload
            (int)dataSize,         // payload size
            0,                      // flags - unused
            (sockaddr const*)&toAddress, // who we're sending to
            sizeof(sockaddr_in));  // size of that structure

        if (size > 0) 
        {
            return size;
        }
    }

    // Not particularly interested in errors - you can 
    // check this though if you want to see if something
    // has happened to your socket.
    return 0;
}

//-----------------------------------------------------------------------------------
void UDPSocket::Bind(const char* address, const char* portNumber)
{
    m_socket = CreateUDPSocket(address, portNumber, &m_address);
}

//-----------------------------------------------------------------------------------
void UDPSocket::Unbind()
{
    NetSystem::CloseSocket(m_socket);
    m_socket = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------
size_t UDPSocket::RecieveFrom(sockaddr_in& fromAddress, void* buffer)
{
    if (m_socket != INVALID_SOCKET)
    {
        // recv will return amount of data read, should always be <= buffer_size
        // Also, if you send, say, 3 KB with send, recv may actually
        // end up returning multiple times (say, 1KB, 512B, and 1.5KB) because 
        // the message got broken up - so be sure you application watches for it

        sockaddr_storage addr;
        int addrlen = sizeof(addr);

        int size = ::recvfrom(m_socket,
            (char*)buffer,    // what we're reading into
            PACKET_MTU,      // max data we can read
            0,                // optional flags (see docs if you're curious)
            (sockaddr*)&addr, // Who sent the message
            &addrlen);       // length of their address

        if (size > 0)
        {
            // We're only doing IPv4 - if we got a non-IPv4 address
            // assume it's garbage
            ASSERT_OR_DIE(addrlen == sizeof(sockaddr_in), "We got a non IPV4 address.");

            memcpy(&fromAddress, &addr, addrlen);
            return size;
        }
    }

    // Again, I don't particularly care about the 
    // error code for now.  It may tell us
    // the guy we're sending to is bad, but we can't really
    // do anything with that yet. 
    return 0U;
}