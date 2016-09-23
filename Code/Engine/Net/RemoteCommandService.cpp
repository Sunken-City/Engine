#include "Engine/Net/RemoteCommandService.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "NetSystem.hpp"
#include <string.h>

RemoteCommandService* RemoteCommandService::instance = nullptr;

//-----------------------------------------------------------------------------------
RemoteCommandService::RemoteCommandService()
    : m_listener(nullptr)
{

}

//-----------------------------------------------------------------------------------
RemoteCommandService::~RemoteCommandService()
{
    if (m_listener)
    {
        delete m_listener;
    }
    for (RemoteCommandServiceConnection* connection : m_connections)
    {
#pragma todo("Close out any connections first")
        delete connection;
    }
    m_connections.clear();
}

//-----------------------------------------------------------------------------------
bool RemoteCommandService::Host(const char* hostName)
{
    UNUSED(hostName)
    m_listener = new TCPListener(REMOTE_COMMAND_SERVICE_PORT_STRING);
    return true;
}

//-----------------------------------------------------------------------------------
bool RemoteCommandService::StopHosting()
{
    ASSERT_OR_DIE(m_listener != nullptr, "Tried to stop hosting without having an existing listener");
#pragma todo("Close out connections here.")
    m_listener->Stop();
    delete m_listener;
    m_listener = nullptr;
    return true;
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::DisconnectFromHost()
{
    ASSERT_OR_DIE(m_connections.size() == 1 && m_connections[0] != nullptr, "Tried to disconnect without a valid connection");
    delete m_connections[0];
    m_connections.clear();
}

//-----------------------------------------------------------------------------------
bool RemoteCommandService::Join(const char* hostName)
{
    RemoteCommandServiceConnection* connection = new RemoteCommandServiceConnection(new TCPConnection(hostName, REMOTE_COMMAND_SERVICE_PORT_STRING));
    connection->m_tcpConnection->m_socket = NetSystem::JoinSocket(hostName, REMOTE_COMMAND_SERVICE_PORT_STRING, &connection->m_tcpConnection->m_address);
    bool isValid = connection->m_tcpConnection->m_socket != INVALID_SOCKET;
    if (isValid)
    {
        AddConnection(connection);
    }
    else
    {
        delete connection;
    }
    return isValid;
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::SendCommand(byte commandId, const char* command)
{
    for (RemoteCommandServiceConnection* connection : m_connections)
    {
        connection->Send(commandId, command);
    }
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::Update()
{
    CheckForConnection(); //Accepts
    CheckForMessages(); //Recieves
    CheckForDisconnection(); //Disconeccts
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::AddConnection(RemoteCommandServiceConnection* connection)
{
    connection->m_onMessage.RegisterMethod(this, &RemoteCommandService::OnRecieveRemoteMessage);
    m_connections.push_back(connection);
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::OnRecieveRemoteMessage(RemoteCommandServiceConnection* , const byte , const char* msg)
{
    Console::instance->PrintLine(Stringf("Running remote command: %s", msg), RGBA::FOREST_GREEN);
    Console::instance->RunCommand(msg);
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::CheckForConnection()
{
    if (m_listener)
    {
        TCPConnection* connection = m_listener->ListenAndAcceptConnection();
        if (connection)
        {
            RemoteCommandServiceConnection* rcs = new RemoteCommandServiceConnection(connection);
            AddConnection(rcs);
            Console::instance->PrintLine(Stringf("Connected with %s", rcs->GetAddressString()), RGBA::GBLIGHTGREEN);
        }
    }
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::CheckForMessages()
{
    for (RemoteCommandServiceConnection* connection : m_connections)
    {
        connection->Receive();
    }
}

//-----------------------------------------------------------------------------------
void RemoteCommandService::CheckForDisconnection()
{

}

//-----------------------------------------------------------------------------------
RemoteCommandServiceConnection::RemoteCommandServiceConnection(TCPConnection* tcpConn)
    : m_tcpConnection(tcpConn)
{

}

//-----------------------------------------------------------------------------------
RemoteCommandServiceConnection::~RemoteCommandServiceConnection()
{
    if (m_tcpConnection && m_tcpConnection->IsConnected())
    {
        m_tcpConnection->Disconnect();
        delete m_tcpConnection;
    }
}

//-----------------------------------------------------------------------------------
void RemoteCommandServiceConnection::Send(byte commandId, const char* command)
{
    m_tcpConnection->Send(&commandId, 1);
    m_tcpConnection->Send(command, strlen(command));
    char nil = NULL;
    m_tcpConnection->Send(&nil, 1);
}

//-----------------------------------------------------------------------------------
void RemoteCommandServiceConnection::Receive()
{
    const size_t BUFFER_SIZE = 1024;
    byte buffer[BUFFER_SIZE];
    size_t read = m_tcpConnection->Receive(buffer, BUFFER_SIZE);
    for (size_t i = 0; i < read; ++i)
    {
        char c = buffer[i];
        m_nextMessage.push_back(c);
        if (c == NULL)
        {
            m_onMessage.Trigger(this, m_nextMessage[0], &m_nextMessage[1]);
            m_nextMessage.clear();
        }
    }
    read = m_tcpConnection->Receive(buffer, BUFFER_SIZE);
}

//-----------------------------------------------------------------------------------
const char* RemoteCommandServiceConnection::GetAddressString()
{
    return this->m_tcpConnection->GetAddressString();
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(cshost)
{
    UNUSED(args)
    if (RemoteCommandService::instance->IsHosting())
    {
        Console::instance->PrintLine("Error: Already hosting.", RGBA::MAROON);
        return;
    }
    if (RemoteCommandService::instance->Host(NetSystem::instance->GetLocalHostName()))
    {
        Console::instance->PrintLine("Now Hosting", RGBA::GBBLACK);
        Console::instance->PrintLine(Stringf("Host: %s:%s, Address: %s", RemoteCommandService::instance->m_listener->m_host, RemoteCommandService::instance->m_listener->m_port, RemoteCommandService::instance->m_listener->GetAddressString()), RGBA::GBLIGHTGREEN);
    }
    else
    {
        Console::instance->PrintLine("Failed to start hosting", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(commandserverstop)
{
    UNUSED(args)
    if (!RemoteCommandService::instance->IsHosting())
    {
        Console::instance->PrintLine("Error: Not currently hosting.", RGBA::MAROON);
        return;
    }
    if (RemoteCommandService::instance->StopHosting())
    {
        Console::instance->PrintLine("Stopped Hosting", RGBA::GBBLACK);
    }
    else
    {
        Console::instance->PrintLine("Failed to stop hosting", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(commandserverinfo)
{
    UNUSED(args)
    if (RemoteCommandService::instance->IsHosting())
    {
        Console::instance->PrintLine("Server State: Hosting", RGBA::GBLIGHTGREEN);
        for (RemoteCommandServiceConnection* conn : RemoteCommandService::instance->m_connections)
        {
            const char* hostname = conn->m_tcpConnection->m_host;
            sockaddr* address = (sockaddr*)&conn->m_tcpConnection->m_address;
            Console::instance->PrintLine(Stringf("Client: %s, Address: %s", hostname, NetSystem::SockAddrToString(address)), RGBA::GBLIGHTGREEN);
        }
        return;
    }
    else if (RemoteCommandService::instance->m_connections.size() == 1)
    {
        Console::instance->PrintLine("Server State: Client", RGBA::GBLIGHTGREEN);

        const char* hostname = RemoteCommandService::instance->m_connections[0]->m_tcpConnection->m_host;
        sockaddr* address = (sockaddr*)&RemoteCommandService::instance->m_connections[0]->m_tcpConnection->m_address;
        Console::instance->PrintLine(Stringf("Host: %s, Address: %s", hostname, NetSystem::SockAddrToString(address)), RGBA::GBLIGHTGREEN);
        return;
    }
    else
    {
        Console::instance->PrintLine("Server State: None", RGBA::GBLIGHTGREEN);
    }

}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(csjoin)
{
    const int IP_INDEX = 0;

    if (RemoteCommandService::instance->IsHosting())
    {
        Console::instance->PrintLine("Failed to join remote session, as you are already hosting", RGBA::RED);
        return;
    }
    else if (RemoteCommandService::instance->IsJoined())
    {
        Console::instance->PrintLine("Failed to join remote session, as you are already in another", RGBA::RED);
        return;
    }

    if (args.HasArgs(0))
    {
        if (RemoteCommandService::instance->Join(NetSystem::GetLocalHostName()))
        {
            Console::instance->PrintLine("Joined remote session", RGBA::GBLIGHTGREEN);
        }
    }
    else if (args.HasArgs(1))
    {
        std::string ip = args.GetStringArgument(IP_INDEX);
        if (RemoteCommandService::instance->Join(ip.c_str()))
        {
            Console::instance->PrintLine("Joined remote session", RGBA::GBLIGHTGREEN);
        }

    }
    else
    {
        Console::instance->PrintLine("Failed to join remote session", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
// CONSOLE_COMMAND(netlisttcpaddresses)
// {
//     if (!args.HasArgs(1))
//     {
//         Console::instance->PrintLine("netlisttcpaddresses <port>", RGBA::GRAY);
//     }
//     else
//     {
//         const int PORT_INDEX = 0;
//         std::string port = args.GetStringArgument(PORT_INDEX);
//         addrinfo* addresses = NetSystem::AllocAddressesForHost(NetSystem::GetLocalHostName(), port.c_str(), AF_INET, SOCK_STREAM, 0);
// 
//         Console::instance->PrintLine(Stringf("Local Name: %s", NetSystem::instance->GetLocalHostName(), port.c_str()), RGBA::GBDARKGREEN);
// 
//     }
// }

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(bcmd)
{
    if (args.HasArgs(0))
    {
        Console::instance->PrintLine("Broadcast Command: Runs a remote command on local machine AND any joined command services", RGBA::GRAY);
        Console::instance->PrintLine("bcmd <command name> <command's arguments>", RGBA::GRAY);
        return;
    }
    for (RemoteCommandServiceConnection* connection : RemoteCommandService::instance->m_connections)
    {
        connection->Send(MSG_COMMAND, args.GetAllArguments().c_str());
    }
    Console::instance->RunCommand(args.GetAllArguments());
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(rcmd)
{
    if (args.HasArgs(0))
    {
        Console::instance->PrintLine("Remote Command: Runs a remote command on any joined command services", RGBA::GRAY);
        Console::instance->PrintLine("rcmd <command name> <command's arguments>", RGBA::GRAY);
        return;
    }
    for (RemoteCommandServiceConnection* connection : RemoteCommandService::instance->m_connections)
    {
        connection->Send(MSG_COMMAND, args.GetAllArguments().c_str());
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(commandserverleave)
{
    UNUSED(args)
    if (RemoteCommandService::instance->IsJoined())
    {
        RemoteCommandService::instance->DisconnectFromHost();
        Console::instance->PrintLine("Successfully disconnected.", RGBA::CHOCOLATE);
    }
    else
    {
        Console::instance->PrintLine("Failed to disconnect because you're not connected to a host.", RGBA::RED);
    }
}