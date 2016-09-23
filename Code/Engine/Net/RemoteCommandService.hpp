#pragma once
#include "Engine/Net/TCPIP/TCPConnection.hpp"
#include "Engine/Net/TCPIP/TCPListener.hpp"
#include "Engine/Core/Event.hpp"
#include <vector>

//TYPEDEFS/////////////////////////////////////////////////////////////////////
typedef unsigned char byte;

//CONSTANTS/////////////////////////////////////////////////////////////////////
#define REMOTE_COMMAND_SERVICE_PORT 4325
#define REMOTE_COMMAND_SERVICE_PORT_STRING "4325"
const byte MSG_COMMAND = 1; //Run a console command
const byte MSG_ECHO = 2; //Print on the remote console
const byte MSG_RENAME = 3; //Give the remote connection a name

//-----------------------------------------------------------------------------------
class RemoteCommandServiceConnection
{
public:
    RemoteCommandServiceConnection(TCPConnection* tcpConnection);
    ~RemoteCommandServiceConnection();
    void Send(byte commandId, const char* command);
    void Receive();
    const char* GetAddressString();
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    TCPConnection* m_tcpConnection;
    std::vector<char> m_nextMessage;
    Event<RemoteCommandServiceConnection*, byte, const char*> m_onMessage;
};

//-----------------------------------------------------------------------------------
class RemoteCommandService
{
public:
    RemoteCommandService();
    ~RemoteCommandService();
    bool Host(const char* hostName); //Create a socket
    bool StopHosting();
    bool Join(const char* hostName); //Create a TCPConnection and add it to the connection list
    void SendCommand(byte commandId, const char* command);
    void Update();
    void AddConnection(RemoteCommandServiceConnection* connection);
    void OnRecieveRemoteMessage(RemoteCommandServiceConnection* connectionPointer, const byte id, const char* msg);
    void CheckForConnection();
    void CheckForMessages();
    void CheckForDisconnection();
    inline bool IsHosting() { return m_listener != nullptr; };
    inline bool IsJoined() { return !IsHosting() && (m_connections.size() > 0); };
    void DisconnectFromHost();
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static RemoteCommandService* instance;

    TCPListener* m_listener;
    std::vector<RemoteCommandServiceConnection*> m_connections;
    Event<RemoteCommandServiceConnection*> m_onConnectionJoin;
    Event<RemoteCommandServiceConnection*> m_onConnectionLeave;
    Event<RemoteCommandServiceConnection*, const byte, const char*> m_onMessage;
};