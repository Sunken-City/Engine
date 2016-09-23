#include "Engine/Net/TCPIP/TCPListener.hpp"
#include "Engine/Net/NetSystem.hpp"
#include <string>

//-----------------------------------------------------------------------------------
TCPListener::TCPListener(const char* host, const char* port, int /*= 1*/)
    : m_socket(NetSystem::CreateListenSocket(host, port, &m_address))
    , m_host(host)
    , m_port(port)
{

}

//-----------------------------------------------------------------------------------
TCPListener::TCPListener(const char* port) : TCPListener(NetSystem::instance->GetLocalHostName(), port)
{

}

//-----------------------------------------------------------------------------------
TCPConnection* TCPListener::ListenAndAcceptConnection()
{
    sockaddr_in address;
    SOCKET socket = NetSystem::AcceptConnection(m_socket, &address);
    if (socket != INVALID_SOCKET)
    {
        TCPConnection* newConnection = new TCPConnection(socket, address);
        return newConnection;
    }
    else
    {
        return nullptr;
    }
}

//-----------------------------------------------------------------------------------
void TCPListener::Stop()
{
    NetSystem::CloseSocket(m_socket);
    m_socket = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------
bool TCPListener::IsListening()
{
    return m_socket != INVALID_SOCKET;
}

const char* TCPListener::GetAddressString()
{
    sockaddr* address = (sockaddr*)&m_address;
    return NetSystem::SockAddrToString(address);
}
