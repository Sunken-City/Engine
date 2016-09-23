#pragma once
#pragma comment(lib, "ws2_32")
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Engine/Net/TCPIP/TCPConnection.hpp"

class TCPListener
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    TCPListener(const char* host, const char* port, int queueCount = 1); //Bind & Listen
    TCPListener(const char* port); //default to localhost (host name ip) 

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    TCPConnection* ListenAndAcceptConnection(); //::accept and creates socket
    void Stop(); //Close Socket
    bool IsListening(); //Store as is_connected
    const char* GetAddressString();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    SOCKET m_socket;
    sockaddr_in m_address;
    const char* m_host;
    const char* m_port;
};