#pragma once
#pragma comment(lib, "ws2_32")
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

class TCPConnection
{
public:
    TCPConnection(SOCKET s, sockaddr_in& my_addr); //Just set
    TCPConnection(const char* host, const char* portNumber); //SocketConnect

    void Disconnect(); //Close Socket
    size_t Send(const void* data, size_t size); //Send On Socket
    size_t Receive(void* buffer, size_t size); //ReceiveOnSocket
    bool IsConnected(); //Socket != INVALID_SOCKET
    const char* GetAddressString();

    //MEMBER VARIABLES////////////////////////////////////////////////////////////////////
    SOCKET m_socket;
    sockaddr_in m_address;
    const char* m_host;
    const char* m_port;
};