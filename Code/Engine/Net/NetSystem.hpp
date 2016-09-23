#pragma once
#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

class NetSystem
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NetSystem();
    ~NetSystem();

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static const char* GetLocalHostName();
    static addrinfo* AllocAddressesForHost(const char* hostName, const char* portNumber, int connectionFamily, int socketType, int flags = 0);
    static SOCKET CreateListenSocket(const char* address, const char* service, sockaddr_in* outAddress);
    static SOCKET AcceptConnection(SOCKET hostSocket, sockaddr_in* outTheirAddress);
    static SOCKET JoinSocket(const char* address, const char* service, sockaddr_in* outAddress);
    static void CloseSocket(SOCKET sock);
    static size_t SendOnSocket(bool& outShouldDisconnect, SOCKET mySocket, const void* data, const size_t dataSize);
    static size_t RecieveFromSocket(bool& outShouldDisconnect, SOCKET mySocket, void* buffer, const size_t bufferSize);
    static const char* SockAddrToString(const sockaddr* address);
    static bool StringToSockAddrIPv4(sockaddr_in& outSockAddr, const char* string);
    static sockaddr_in StringToSockAddrIPv4(const char* ip, const uint16_t port);
    static void* GetInAddr(const sockaddr* socketAddress);
    static bool SocketErrorShouldDisconnect(const int32_t error);
    static void FreeAddresses(addrinfo *addresses);
    static sockaddr* GetLocalHostAddressUDP(const char* portNumber);
    static bool SockaddrCompare(const sockaddr_in& first, const sockaddr_in& second);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static NetSystem* instance;
};