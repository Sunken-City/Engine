#pragma once
#pragma comment(lib, "ws2_32")
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PACKET_MTU 1232

class UDPSocket
{
public:
    UDPSocket() : m_socket(INVALID_SOCKET) {};
    ~UDPSocket() {};
    void Bind(const char* address, const char* portNumber);
    void Unbind();
    bool IsBound() { return m_socket != INVALID_SOCKET; };
    size_t SendTo(const sockaddr_in& toAddress, void const* data, const size_t dataSize);
    size_t RecieveFrom(sockaddr_in& fromAddress, void* buffer);
    //You get back an address of who sent the data. 
    //You ask for max length, and if you get more than you asked for, you get the error
    //Regardless of how much you ask for, you simply get one packet.

    SOCKET m_socket;
    sockaddr_in m_address;

private:
    SOCKET CreateUDPSocket(char const *addr, char const *service, sockaddr_in *out_addr);
};