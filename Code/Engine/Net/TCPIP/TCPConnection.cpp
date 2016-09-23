#include "Engine/Net/TCPIP/TCPConnection.hpp"
#include "Engine/Net/NetSystem.hpp"

//-----------------------------------------------------------------------------------
TCPConnection::TCPConnection(SOCKET socket, sockaddr_in& inAddress)
    : m_socket(socket)
    , m_address(inAddress)
    , m_host(NetSystem::SockAddrToString((sockaddr*)&inAddress))
{
}

//-----------------------------------------------------------------------------------
TCPConnection::TCPConnection(const char* host, const char* portNumber)
    : m_host(host)
    , m_port(portNumber)
{

}

//-----------------------------------------------------------------------------------
void TCPConnection::Disconnect()
{
    NetSystem::CloseSocket(m_socket);
    m_socket = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------
size_t TCPConnection::Send(const void* data, size_t size)
{
    bool shouldDisconnect = false;
    size_t sizeSent = NetSystem::SendOnSocket(shouldDisconnect, m_socket, data, size);
    return sizeSent;
}

//-----------------------------------------------------------------------------------
size_t TCPConnection::Receive(void* buffer, size_t size)
{
    bool shouldDisconnect = false;
    size_t sizeRecieved = NetSystem::RecieveFromSocket(shouldDisconnect, m_socket, buffer, size);
    return sizeRecieved;
}

//-----------------------------------------------------------------------------------
bool TCPConnection::IsConnected()
{
    return m_socket != INVALID_SOCKET;
}

//-----------------------------------------------------------------------------------
const char* TCPConnection::GetAddressString()
{
    sockaddr* address = (sockaddr*)&m_address;
    return NetSystem::SockAddrToString(address);
}
