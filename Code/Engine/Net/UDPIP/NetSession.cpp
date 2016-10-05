#include "Engine/Net/UDPIP/NetSession.hpp"
#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Net/UDPIP/NetPacket.hpp"
#include "Engine/Core/Event.hpp"
#include "Engine/Input/Logging.hpp"
#include "Engine/Time/Time.hpp"

NetSession* NetSession::instance = nullptr;
extern Event<float> NetworkUpdate;
extern Event<> NetworkCleanup;

//-----------------------------------------------------------------------------------
NetSession::NetSession(float tickRatePerSecond) 
    : m_myConnection(nullptr)
    , m_hostConnection(nullptr)
    , m_tickRate(tickRatePerSecond)
    , m_timeLastJoinRequestSent(0.0f)
    , m_timeSinceLastUpdate(0.0f)
    , m_numConnections(0)
    , m_sessionState(State::INVALID)
    , m_lastError(ErrorCode::NONE)
    , m_timeoutEnabled(false)
    , m_sessionInfoText(nullptr)
    , m_netLagText(nullptr)
    , m_netLossText(nullptr)
    , m_connectionCountText(nullptr)
    , m_isListening(true)
{
    m_packetChannel.m_additionalLagMilliseconds = 0;//Range<double>(50, 150);
    m_packetChannel.m_dropRate = 0.0f;//0.1f;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        m_allConnections[i] = nullptr;
    }
}

//-----------------------------------------------------------------------------------
NetSession::~NetSession()
{
    for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
    {
        Disconnect(m_allConnections[i]);
    }
}

//-----------------------------------------------------------------------------------
bool NetSession::Start(const char* portNumber)
{
    ASSERT_OR_DIE(m_sessionState == State::INVALID, "Attempted to start the session after starting it once");
    m_packetChannel.Bind(NetSystem::GetLocalHostName(), portNumber);
    if (!m_packetChannel.IsBound())
    {
        m_lastError = ErrorCode::START_FAILED_TO_CREATE_SOCKET;
    }
    SetSessionState(State::DISCONNECTED);
    OnEnterDisconnectedState();
    NetworkUpdate.RegisterMethod(NetSession::instance, &NetSession::Update);
    return m_packetChannel.IsBound();
}

//-----------------------------------------------------------------------------------
void NetSession::Stop()
{
    if (m_sessionState == CONNECTED)
    {
        Leave();
    }
    m_packetChannel.Unbind();
    NetworkUpdate.UnregisterMethod(this, &NetSession::Update);
    SetSessionState(State::INVALID);
}

//-----------------------------------------------------------------------------------
void NetSession::Cleanup()
{
    Console::instance->PrintLine("Shutting down net session.", RGBA::CORNFLOWER_BLUE);
    if (m_sessionState != State::INVALID)
    {
        Stop();
    }
    delete NetSession::instance;
    NetSession::instance = nullptr;
}

//-----------------------------------------------------------------------------------
bool NetSession::SetSessionState(State newState, void(NetSession::*onStateSwitchCallback)())
{
    if (m_sessionState != newState)
    {
        DebuggerPrintf("Changed State from %s to %s\n", GetStateCstr(m_sessionState), GetStateCstr(newState));
        m_sessionState = newState;

        //If we have any one-shot functions that were waiting for a callback on state switch, fire them off now and register the new one.
        m_OnStateSwitch.Trigger();
        m_OnStateSwitch.UnregisterAllSubscriptions();
        if (onStateSwitchCallback)
        {
            m_OnStateSwitch.RegisterMethod(this, onStateSwitchCallback);
        }
        return true;
    }
    else
    {
        ERROR_RECOVERABLE("State machine was set to the same state.");
        return false;
    }
}

//-----------------------------------------------------------------------------------
void NetSession::Update(float deltaSeconds)
{
    ProcessIncomingPackets(); 
    m_timeSinceLastUpdate += deltaSeconds;
    if (m_timeSinceLastUpdate >= m_tickRate)
    {
        for (NetConnection* conn : m_allConnections)
        {
            if (conn)
            {
                m_OnNetTick.Trigger(conn);
                conn->ConstructAndSendPacket();
            }
        }
        m_timeSinceLastUpdate = 0.0f;
    }
    CheckForTimeouts();
    CheckForJoinResponse();
}

//-----------------------------------------------------------------------------------
void NetSession::ProcessIncomingPackets(const size_t maxPacketsToProcess)
{
    UNUSED(maxPacketsToProcess)
    NetPacket packet(INVALID_CONNECTION_INDEX);
    NetSender from;
    from.session = this;
    size_t read = m_packetChannel.RecieveFrom(from.address, packet.m_buffer);
    while (read > 0)
    {
        packet.SetReadableBytes(read);
        ProcessIncomingPacket(from, packet);
        read = m_packetChannel.RecieveFrom(from.address, packet.m_buffer);
    }
}

//-----------------------------------------------------------------------------------
void NetSession::ProcessIncomingPacket(NetSender& from, NetPacket& packet)
{
    packet.ReadHeader();

    //Get the connection this packet came from, if it did.
    from.connection = GetConnection(packet.m_header.fromConnectionIndex);

    //Process the messages in the packet.
    uint8_t numMessages = packet.m_header.messageCount;
    NetMessage msg;
    //Console::instance->PrintLine(Stringf("Received Packet with %i messages", numMessages), RGBA::VAPORWAVE);
    for (uint8_t i = 0; i < numMessages; ++i)
    {
        packet.ReadMessage(msg);

        // Make sure we can process it
        // - Does it require a connection?
        // - If we have a connection, does the connection want to process this message?
        // - etc...
        if (CanProcessMessage(from, msg))
        {
            if (from.connection != nullptr)
            {
                from.connection->ProcessMessage(from, msg);
            }
            else
            {
                msg.Process(from);
                if (msg.IsReliable())
                {
                    from.connection = GetConnection(GetConnectionIndexFromAddress(from.address));
                }
            }
        }
    }
    if (GetConnection(packet.m_header.fromConnectionIndex) && m_myConnection && m_myConnection->m_index != INVALID_CONNECTION_INDEX)
    {
        from.connection->MarkPacketReceived(packet);
    }
}

//-----------------------------------------------------------------------------------
void NetSession::RegisterMessage(uint8_t type, const char* messageName, NetMessageCallback* functionPointer, uint32_t optionFlags, uint32_t controlFlags)
{
    ASSERT_OR_DIE(m_sessionState == State::INVALID, "Attempted to register a message after the session was initialized");
    ASSERT_OR_DIE(m_netMessageDefinitions[type].callbackFunction == nullptr, "Attempted to overwrite an existing message definition");

    m_netMessageDefinitions[type].id = type;
    m_netMessageDefinitions[type].debugName = messageName;
    m_netMessageDefinitions[type].callbackFunction = functionPointer;
    m_netMessageDefinitions[type].m_optionFlags = optionFlags;
    m_netMessageDefinitions[type].m_controlFlags = controlFlags;
}

//-----------------------------------------------------------------------------------
void NetSession::SendMessageDirect(const sockaddr_in& to, const NetMessage& msg)
{
#pragma todo ("If the message requires a connection, fail here")
    if (!IsRunning())
    {
        return;
    }
    NetPacket packet(GetMyConnectionIndex());
    packet.WriteMessageAndFinalize(msg);
    m_packetChannel.SendTo(to, packet.m_buffer, packet.GetTotalReadableBytes());
}

//-----------------------------------------------------------------------------------
size_t NetSession::SendMessagesDirect(sockaddr_in& to, NetMessage** messages, size_t numMessages)
{
#pragma todo ("If the message requires a connection, fail here")
    if (!IsRunning())
    {
        return 0;
    }
    NetPacket packet(GetMyConnectionIndex());
    size_t successfullyPacked = packet.WriteMessagesAndFinalize(messages, numMessages);
    m_packetChannel.SendTo(to, packet.m_buffer, packet.GetTotalReadableBytes());
    return successfullyPacked;
}

//-----------------------------------------------------------------------------------
bool NetSession::IsRunning()
{
    return m_packetChannel.IsBound();
}

//-----------------------------------------------------------------------------------
bool NetSession::IsHost()
{
    return m_hostConnection && m_hostConnection == m_myConnection;
}

//-----------------------------------------------------------------------------------
const NetMessageDefinition* NetSession::FindDefinition(byte messageType)
{
    NetMessageDefinition* def = &m_netMessageDefinitions[messageType];
    if (def->callbackFunction != nullptr)
    {
        return def;
    }
    else
    {
        return nullptr;
    }
}

//-----------------------------------------------------------------------------------
NetConnection* NetSession::CreateConnection(uint8_t index, const char* guid, sockaddr_in address)
{
    if (GetConnection(index) != nullptr)
    {
        return nullptr;
    }

    NetConnection* connection = new NetConnection(index, guid, address, this);
    if (!Connect(connection, index))
    {
        return nullptr;
    }
    if (connection->IsMyConnection())
    {
        m_myConnection = connection;
        connection->m_state = NetConnection::State::LOCAL;
    }
    else
    {
        connection->m_state = NetConnection::State::UNCONFIRMED;
    }

    ++m_numConnections;
    return connection;
}

//-----------------------------------------------------------------------------------
NetConnection* NetSession::GetConnection(uint8_t index)
{
    if (index < MAX_CONNECTIONS)
    {
        return m_allConnections[index];
    }
    else
    {
        return nullptr;
    }
}

//-----------------------------------------------------------------------------------
void NetSession::DestroyConnection(uint8_t index)
{
    NetConnection* conn = GetConnection(index);
    if (conn == nullptr)
    {
        return;
    }
    if (conn->IsMyConnection())
    {
        m_myConnection = nullptr;
    }
    if (conn->IsHostConnection())
    {
        m_hostConnection = nullptr;
    }

    m_OnConnectionLeave.Trigger(conn);
    delete conn;
    --m_numConnections;
    m_allConnections[index] = nullptr;
}

//-----------------------------------------------------------------------------------
uint8_t NetSession::GetMyConnectionIndex()
{
    if (m_myConnection)
    {
        return m_myConnection->m_index;
    }
    else
    {
        return INVALID_CONNECTION_INDEX;
    }
}

//-----------------------------------------------------------------------------------
uint8_t NetSession::GetConnectionIndexFromAddress(const sockaddr_in& address)
{
    for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (m_allConnections[i] && NetSystem::SockaddrCompare(m_allConnections[i]->m_address, address))
        {
            return i;
        }
    }
    return INVALID_CONNECTION_INDEX;
}

//-----------------------------------------------------------------------------------
void OnPingReceived(const NetSender& sender, NetMessage& msg)
{
    const char* str = msg.ReadString();

    Console::instance->PrintLine(Stringf("Ping Receieved from %s. [%s]", NetSystem::SockAddrToString((sockaddr*)&sender.address), ((nullptr != str) ? str : "null")), RGBA::FOREST_GREEN);

    NetMessage pong(NetMessage::PONG);
    NetSession::instance->SendMessageDirect(sender.address, pong);
}

//-----------------------------------------------------------------------------------
void OnPongReceived(const NetSender& sender, NetMessage&)
{
    Console::instance->PrintLine(Stringf("Pong received from %s.", NetSystem::SockAddrToString((sockaddr*)&sender.address)), RGBA::GBWHITE);
}

//-----------------------------------------------------------------------------------
void OnHeartbeatReceived(const NetSender& sender, NetMessage& msg)
{
    Console::instance->PrintLine(Stringf("[%i] Heartbeat received from %s. <3", msg.m_reliableId, NetSystem::SockAddrToString((sockaddr*)&sender.address)), RGBA::VAPORWAVE);
}

//-----------------------------------------------------------------------------------
void OnJoinAcceptReceived(const NetSender& sender, NetMessage& msg)
{
    if (sender.session->GetSessionState() != NetSession::State::JOINING)
    {
        ERROR_RECOVERABLE("OnJoinAccept was Received, but we aren't in the joining state!");
    }
//     if (nuonce doesn't match)
//     {
//         return;
//     }
    
    Console::instance->PrintLine(Stringf("Joined the host at %s.", NetSystem::SockAddrToString((sockaddr*)&sender.address)), RGBA::VAPORWAVE);
#pragma todo("ReadConnectionInfo function would simplify this")

    NetConnection* host = sender.session->GetHostConnection();
    const char* hostGuid = msg.ReadString();
    memcpy(host->m_guid, hostGuid, strlen(hostGuid));
    msg.Read<uint8_t>(host->m_index);

    NetConnection* me = sender.session->GetMyConnection();
    ASSERT_OR_DIE(strcmp(me->m_guid, msg.ReadString()) == 0, "Got back a different guid, potentially corrputed packet detected");
    msg.Read<uint8_t>(me->m_index);

    sender.session->Connect(me, me->m_index);
    sender.session->SetSessionState(NetSession::State::CONNECTED);
}

//-----------------------------------------------------------------------------------
void OnConnectionLeaveReceived(const NetSender& sender, NetMessage&)
{
    sender.session->Disconnect(sender.connection);
}

//-----------------------------------------------------------------------------------
void OnJoinDenyReceived(const NetSender& sender, NetMessage& msg)
{
    if (sender.session->GetSessionState() != NetSession::JOINING)
    {
        ERROR_RECOVERABLE("OnJoinDeny was Received, but we aren't in the joining state!");
        return;
    }
//     if (nuonce doesn't match)
//     {
//         return;
//     }

    NetSession::ErrorCode code = NetSession::ErrorCode::NONE;
    msg.Read<NetSession::ErrorCode>(code);
    Console::instance->PrintLine(Stringf("Failed to join the host at %s.", NetSystem::SockAddrToString((sockaddr*)&sender.address)), RGBA::RED);
    Console::instance->PrintLine(Stringf("Reason: %s", NetSession::GetErrorCodeCstr(code)), RGBA::VAPORWAVE);
    sender.session->m_lastError = code;

    sender.session->SetSessionState(NetSession::DISCONNECTED);
    sender.session->OnEnterDisconnectedState();
}


//-----------------------------------------------------------------------------------
void OnJoinRequestReceived(const NetSender& sender, NetMessage& msg)
{
    NetSession* sp = sender.session;
    const char* guid = msg.ReadString();

    if (sp->m_myConnection != sp->m_hostConnection)
    {
        sp->SendDeny(NetSession::ErrorCode::JOIN_DENIED_NOT_HOST, sender.address);
        return;
    }
    if (!sp->m_isListening)
    {
        sp->SendDeny(NetSession::ErrorCode::JOIN_DENIED_NOT_ACCEPTING_NEW_CONNECTIONS, sender.address);
        return;
    }
    if (sp->IsPartyFull())
    {
        sp->SendDeny(NetSession::ErrorCode::JOIN_DENIED_FULL, sender.address);
        return;
    }
    if (sp->IsGuidInUse(guid))
    {
        if (!sp->HasConnectionFor(sender.address))
        {
            sp->SendDeny(NetSession::ErrorCode::JOIN_DENIED_GUID_IN_USE, sender.address);
        }
        return;
    }

    NetConnection* cp = sp->CreateConnection(sp->GetNextAvailableIndex(), guid, sender.address);
    NetMessage accept(NetMessage::CoreMessageTypes::JOIN_ACCEPT);
#pragma todo("Make this a 'writeConnInfo' function")
    accept.WriteString(sp->GetHostConnection()->m_guid);
    accept.Write<uint8_t>(sp->GetHostConnection()->m_index);
    accept.WriteString(cp->m_guid);
    accept.Write<uint8_t>(cp->m_index);
    cp->SendMessage(accept);
}

//-----------------------------------------------------------------------------------
void NetSession::SendDeny(ErrorCode reason, const sockaddr_in& address)
{
    NetMessage deny(NetMessage::CoreMessageTypes::JOIN_DENY);
    //deny.write<uint32_t>(nuonce);
    deny.Write<uint8_t>(reason);
    SendMessageDirect(address, deny);
}

//-----------------------------------------------------------------------------------
void NetSession::UpdateNetDebug()
{
    if (!NetSession::instance)
    {
        ShutdownNetDebug();
        return;
    }
    if (m_sessionInfoText)
    {
        sockaddr_in addr = m_packetChannel.GetAddress();
        const sockaddr* address = m_packetChannel.IsBound() ? (const sockaddr*)&addr : nullptr;
        m_sessionInfoText->text = Stringf("Session bound to [%s] - State: %s", NetSystem::SockAddrToString(address), GetStateCstr(m_sessionState));
    }
    if (m_netLagText)
    {
        m_netLagText->text = Stringf("Simulated Net Lag: %.0fms ~ %.0fms", m_packetChannel.m_additionalLagMilliseconds.minValue, m_packetChannel.m_additionalLagMilliseconds.maxValue);
    }
    if (m_netLossText)
    {
        m_netLossText->text = Stringf("Simulated Net Loss: %.2f%%", m_packetChannel.m_dropRate * 100.0f);
    }
    if (m_connectionCountText)
    {
        m_connectionCountText->text = Stringf("Connection Count: %i/%i", m_numConnections, MAX_CONNECTIONS);
    }
    if (m_connectionsText.size() > 0)
    {
        for (unsigned int i = 0; i < NetSession::MAX_CONNECTIONS; ++i)
        {
            ColoredText* textLine = m_connectionsText[i];
            NetConnection* conn = GetConnection((uint8_t)i);
            if (textLine)
            {
                if (conn)
                {
                    textLine->text = Stringf("%s%s[%i %s] %s <%s> lRcv[%.0fms] lSnd[%.0fms] sAck[%i] cAck[%i]",
                        conn->IsMyConnection() ? "*" : " ",
                        conn->IsHostConnection() ? "H" : " ",
                        i,
                        NetSystem::SockAddrToString((const sockaddr*)&conn->m_address),
                        conn->m_guid,
                        conn->GetStateCstr(),
                        conn->m_lastRecievedTimeMs,
                        conn->m_lastSentTimeMs,
                        conn->GetLastSentAck(),
                        conn->GetMostRecentConfirmedAck());
                }
                else
                {
                    textLine->text = Stringf("  [%i] No Connection", i);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void NetSession::ShutdownNetDebug()
{
    m_sessionInfoText = nullptr;
    m_netLagText = nullptr;
    m_netLossText = nullptr;
    m_connectionCountText = nullptr;
    for (unsigned int i = 0; i < m_connectionsText.size(); ++i)
    {
        m_connectionsText[i] = nullptr;
    }
}

//-----------------------------------------------------------------------------------
void NetSession::Host(const char* username)
{
    const int MAX_CHARACTERS = 32;
    char portString[MAX_CHARACTERS];
    ASSERT_OR_DIE(m_sessionState == DISCONNECTED, "Wasn't in a valid state before hosting");
    SetSessionState(HOSTING);
    _itoa_s(htons(m_packetChannel.GetAddress().sin_port), portString, 10);
    sockaddr_in localAddress = *(sockaddr_in*)NetSystem::GetLocalHostAddressUDP(portString);
    m_hostConnection = CreateConnection(0, username, localAddress);
    m_myConnection = m_hostConnection;
    SetSessionState(CONNECTED);
}

//-----------------------------------------------------------------------------------
void NetSession::Join(const char* username, sockaddr_in& hostAddress)
{
    SetSessionState(JOINING);
    m_hostConnection = CreateConnection(0, "hostDefault", hostAddress);
    m_myConnection = new NetConnection(INVALID_CONNECTION_INDEX, username, GetAddress(), this);
    m_timeLastJoinRequestSent = GetCurrentTimeMilliseconds();
    NetMessage request(NetMessage::CoreMessageTypes::JOIN_REQUEST);
    request.WriteString(username);
    m_hostConnection->SendMessage(request);
    //SendMessageDirect(hostAddress, request);
}

//-----------------------------------------------------------------------------------
void NetSession::Leave()
{
    ASSERT_OR_DIE(m_sessionState == CONNECTED, "Wasn't connected before leaving");
    NetMessage leaveMessage(NetMessage::CoreMessageTypes::CONNECTION_LEAVE);
    for (NetConnection* conn : m_allConnections)
    {
        if (conn && conn != m_myConnection)
        {
            SendMessageDirect(conn->m_address, leaveMessage);
        }
    }

    Disconnect(m_myConnection);
    ASSERT_OR_DIE(m_myConnection == nullptr, "Didn't actually disconnect");

    SetSessionState(DISCONNECTED);
    OnEnterDisconnectedState();
}

//------------------------------------------------------------------------
bool NetSession::Connect(NetConnection* cp, const uint8_t idx)
{
    //This guy better not be connected, and this slot needs to be free
    ASSERT_OR_DIE(!cp->IsConnected(), "Attempted to reconnect a connected connection");
    if (GetConnection(idx) != nullptr)
    {
        return false;
    }

    cp->m_index = idx;
    m_allConnections[idx] = cp;

    // If you're the host, and in a P2P environment
    // you would tell everyone else about this connection

    // Only trigger callbacks if MY connection is connected
    if (AmIConnected()) 
    {
        if (cp->IsMyConnection()) 
        {
            // Host always goes first
            if (!cp->IsHostConnection()) 
            {
                m_OnConnectionJoin.Trigger(m_hostConnection);
            }
        }

        // Next do me, or if it's not me, the connection as normal
        m_OnConnectionJoin.Trigger(cp);

        // Finally, if this is me, trigger everyone else
        if (cp->IsMyConnection()) 
        {
            for (size_t i = 0; i < MAX_CONNECTIONS; ++i) 
            {
                NetConnection* otherConnection = m_allConnections[i];
                if ((nullptr != otherConnection) && (cp != otherConnection) && !otherConnection->IsHostConnection()) 
                {
                    m_OnConnectionJoin.Trigger(otherConnection);
                }
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------------
void NetSession::Disconnect(uint8_t index)
{
    Disconnect(m_allConnections[index]);
}

//------------------------------------------------------------------------
// This function has a lot of twiddling involved to give me
// a consistent disconnect order.
void NetSession::Disconnect(NetConnection* cp)
{
    if (cp == nullptr) 
    {
        return;
    }

    uint8_t const idx = cp->m_index;
    ASSERT_OR_DIE(m_allConnections[idx] == cp, "Passed a nonexistant connection to disconnect");

    // Not connected - just remove it silently.
    if (!AmIConnected()) 
    {
        DestroyConnection(idx);
        return;
    }

    // If we're disconnecting the host, and that's NOT ME
    // treat it as if we were disconnecting me.
    // This enforces a consistent disconnect order
    // (everyone else, me, and then the host)
    if (cp->IsHostConnection() && !cp->IsMyConnection()) 
    {
        // This disconnectd me - but if you want to support 
        // migration, then this instead is where you kick off the 
        // the migration event (everyone should get this event 
        // around the same time, and the vote procedures can commence)
        Disconnect(GetMyConnection());
        return;
    }

    // At this point - I know I am connected, so can proceed as normal.
    if (cp->IsMyConnection()) 
    {
        // Disconnect everyone else in the session that is NOT the host
        // and NOT me.
        for (size_t i = 0; i < MAX_CONNECTIONS; ++i) 
        {
            NetConnection* other_cp = m_allConnections[i];
            if ((nullptr != other_cp) && !other_cp->IsMyConnection() && !other_cp->IsHostConnection()) 
            {
                Disconnect(other_cp);
            }
        }
    }

    // Disconnect the guy, or me, if that is the case.
    // Condition of event is connection is removed from array, but before destroyed
    DestroyConnection(idx);

    // I'm disconnecting myself, and I'm not the host, finally disconnect the host last
    if (cp->IsMyConnection() && (m_hostConnection != nullptr) && !cp->IsHostConnection()) 
    {
        DestroyConnection(m_hostConnection->m_index);
        ASSERT_OR_DIE(m_hostConnection == nullptr, "Failed to set the host connection ptr back to nullptr");
    }
}

//-----------------------------------------------------------------------------------
void NetSession::CheckForTimeouts()
{
    if (!m_timeoutEnabled)
    {
        return;
    }
    for (NetConnection* conn : m_allConnections)
    {
        if (conn && !conn->IsMyConnection())
        {
            double msSinceLastContact = GetCurrentTimeMilliseconds() - conn->m_lastRecievedTimeMs;
            if (msSinceLastContact >= NetConnection::BAD_CONNECTION_TIME_MS)
            {
                conn->m_state = NetConnection::State::BAD;
            }
            if (msSinceLastContact >= NetConnection::TIMEOUT_TIME_MS)
            {
                Disconnect(conn);
            }
        }
    }
}

//-----------------------------------------------------------------------------------
const char* NetSession::GetStateCstr(const NetSession::State& state)
{
    switch (state)
    {
    case State::INVALID:
        return "Invalid";
    case State::DISCONNECTED:
        return "Disconnected";
    case State::HOSTING:
        return "Hosting";
    case State::JOINING:
        return "Joining";
    case State::CONNECTED:
        return "Connected";
    default:
        ERROR_AND_DIE("Invalid state passed");
    }
}

//-----------------------------------------------------------------------------------
void NetSession::OnEnterDisconnectedState()
{
    for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
    {
        Disconnect(m_allConnections[i]);
    }
    //Check to make sure we aren't leaking a placeholder connection
    if (m_myConnection && m_myConnection->m_index == INVALID_CONNECTION_INDEX)
    {
        delete m_myConnection;
    }
    m_myConnection = nullptr;
    m_hostConnection = nullptr;
}

//-----------------------------------------------------------------------------------
bool NetSession::HasConnectionFor(const sockaddr_in& address)
{
    for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
    {
        NetConnection* conn = m_allConnections[i];
        if (conn && NetSystem::SockaddrCompare(address, conn->m_address) == 0)
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool NetSession::IsPartyFull()
{
    return m_numConnections >= MAX_CONNECTIONS;
}

//-----------------------------------------------------------------------------------
bool NetSession::IsGuidInUse(const char* guid)
{
    for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
    {
        NetConnection* conn = m_allConnections[i];
        if (conn && strcmp(guid, conn->m_guid) == 0)
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------------
uint8_t NetSession::GetNextAvailableIndex()
{
    for (uint8_t i = 0; i < MAX_CONNECTIONS; ++i)
    {
        NetConnection* conn = m_allConnections[i];
        if (!conn)
        {
            return i;
        }
    }
    return INVALID_CONNECTION_INDEX;
}

//-----------------------------------------------------------------------------------
void NetSession::CheckForJoinResponse()
{
    if (m_sessionState != JOINING)
    {
        return;
    }
    if (GetCurrentTimeMilliseconds() - m_timeLastJoinRequestSent >= NetConnection::TIMEOUT_TIME_MS)
    {
        m_lastError = JOIN_ERROR_HOST_TIMEOUT;
        Console::instance->PrintLine(Stringf("Failed to join the host. Reason: %s", NetSession::GetErrorCodeCstr(m_lastError)), RGBA::RED);
        SetSessionState(DISCONNECTED);
        OnEnterDisconnectedState();
    }
}

//-----------------------------------------------------------------------------------
bool NetSession::AmIConnected()
{
    return m_myConnection != nullptr && m_myConnection->IsConnected();
}

//-----------------------------------------------------------------------------------
bool NetSession::IsMyConnectionConfirmed()
{
    return m_myConnection != nullptr && m_myConnection->m_state == NetConnection::State::CONFIRMED;
}

//-----------------------------------------------------------------------------------
const char* NetSession::GetErrorCodeCstr(const NetSession::ErrorCode& code)
{
    switch (code)
    {
    case NONE:
        return "No Error";
    case START_FAILED_TO_CREATE_SOCKET:
        return "Start failed to return socket";
    case JOIN_ERROR_HOST_TIMEOUT:
        return "Join Error: Host timed out";
    case JOIN_DENIED_NOT_ACCEPTING_NEW_CONNECTIONS:
        return "Join Error: Host not accepting new connections";
    case JOIN_DENIED_NOT_HOST:
        return "Join Error: Connection wasn't the host";
    case JOIN_DENIED_FULL:
        return "Join Error: Host was full";
    case JOIN_DENIED_GUID_IN_USE:
        return "Join Error: Your guid is already in use in that session";
    case ERROR_HOST_DISCONNECTED:
        return "Host Disconnected";
    default:
        ERROR_AND_DIE("Invalid code passed into GetErrorCodeCstr");
    }
}

//-----------------------------------------------------------------------------------
bool NetSession::CanProcessMessage(const NetSender& from, const NetMessage& msg) const
{
    if (from.connection == nullptr)
    {
        return !msg.RequiresConnection();
    }
    else
    {
        return from.connection->CanProcessMessage(msg);
    }
}

//-----------------------------------------------------------------------------------
void NetMessageDefinition::SetOptionFlag(NetMessage::Option flag)
{
    m_optionFlags |= (uint8_t)flag;
}

//-----------------------------------------------------------------------------------
void NetMessageDefinition::SetControlFlag(NetMessage::Control flag)
{
    m_controlFlags |= (uint8_t)flag;
}

//-----------------------------------------------------------------------------------
bool NetMessageDefinition::HasOptionFlag(NetMessage::Option flag) const
{
    return(m_optionFlags & (uint8_t)flag) != 0;
}

//-----------------------------------------------------------------------------------
bool NetMessageDefinition::HasControlFlag(NetMessage::Control flag) const
{
    return(m_controlFlags & (uint8_t)flag) != 0;
}

//-----------------------------------------------------------------------------------
void NetMessageDefinition::ClearOptionFlag(NetMessage::Option flag)
{
    m_optionFlags &= ~(uint8_t)flag;
}

//-----------------------------------------------------------------------------------
void NetMessageDefinition::ClearControlFlag(NetMessage::Control flag)
{
    m_controlFlags &= ~(uint8_t)flag;
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nsinit)
{
    UNUSED(args)
    if (nullptr != NetSession::instance)
    {
        Console::instance->PrintLine("Net session already initialized.", RGBA::ORANGE);
        return;
    }

    NetSession::instance = new NetSession(1.0f/60.0f);
    NetworkCleanup.RegisterMethod(NetSession::instance, &NetSession::Cleanup);

    // Setup
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::PING, "ping", &OnPingReceived, (uint32_t)NetMessage::Option::UNRELIABLE, (uint32_t)NetMessage::Control::PROCESS_CONNECTIONLESS);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::PONG, "pong", &OnPongReceived, (uint32_t)NetMessage::Option::UNRELIABLE, (uint32_t)NetMessage::Control::PROCESS_CONNECTIONLESS);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::HEARTBEAT, "<3", &OnHeartbeatReceived, (uint32_t)NetMessage::Option::RELIABLE, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::INORDER_HEARTBEAT, "Inorder<3", &OnHeartbeatReceived, (uint32_t)NetMessage::Option::RELIABLE | (uint32_t)NetMessage::Option::INORDER, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::JOIN_REQUEST, "joinRequest", &OnJoinRequestReceived, (uint32_t)NetMessage::Option::RELIABLE, (uint32_t)NetMessage::Control::PROCESS_CONNECTIONLESS);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::JOIN_ACCEPT, "joinAccept", &OnJoinAcceptReceived, (uint32_t)NetMessage::Option::RELIABLE | (uint32_t)NetMessage::Option::INORDER, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::JOIN_DENY, "joinDeny", &OnJoinDenyReceived, (uint32_t)NetMessage::Option::UNRELIABLE, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)NetMessage::CONNECTION_LEAVE, "connectionLeave", &OnConnectionLeaveReceived, (uint32_t)NetMessage::Option::UNRELIABLE, (uint32_t)NetMessage::Control::PROCESS_CONNECTIONLESS);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nsstart)
{
    UNUSED(args)
    if (NetSession::instance && NetSession::instance->Start(GAME_PORT_STR))
    {
        // Log Success and Connection Address
        Console::instance->PrintLine(Stringf("Successfully created session at [%s]", NetSystem::SockAddrToString((sockaddr*)&NetSession::instance->m_packetChannel.m_socket.m_address)), RGBA::BADDAD);
    }
    else 
    {
        Console::instance->PrintLine("Failed to bring the net session online.", RGBA::RED);
        delete NetSession::instance;
        NetSession::instance = nullptr;
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nscleanup)
{
    UNUSED(args)
    if (nullptr == NetSession::instance)
    {
        Console::instance->PrintLine("Net session isn't running.", RGBA::ORANGE);
        return;
    }

    NetSession::instance->Cleanup();
    NetworkCleanup.UnregisterMethod(NetSession::instance, &NetSession::Cleanup);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nsstop)
{
    UNUSED(args)
    if (nullptr == NetSession::instance)
    {
        Console::instance->PrintLine("Net session isn't running.", RGBA::ORANGE);
        return;
    }

    NetSession::instance->Stop();
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(ping)
{
    if (!(args.HasArgs(0) || args.HasArgs(1) || args.HasArgs(2)))
    {
        Console::instance->PrintLine("ping <ip address> [optional message]", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        // so it's expecting a full address string,
        // ex: 192.168.52.127:4334
        sockaddr_in to;
        if (args.HasArgs(0))
        {
            to = *(sockaddr_in*)NetSystem::GetLocalHostAddressUDP(GAME_PORT_STR);
        }
        else
        {
            std::string fullIPString = args.GetStringArgument(0);
            size_t portLocation = fullIPString.find(":");
            if (portLocation == fullIPString.npos)
            {
                to = NetSystem::StringToSockAddrIPv4(fullIPString.c_str(), GAME_PORT);
            }
            else
            {
                std::string ipPart = fullIPString.substr(0, portLocation);
                std::string portPart = fullIPString.substr(portLocation + 1, fullIPString.length() - portLocation);
                to = NetSystem::StringToSockAddrIPv4(ipPart.c_str(), (uint16_t)std::stoi(portPart));
            }
        }

        NetMessage msg(NetMessage::PING);
        const char* optionalMessage = nullptr;
        std::string message;
        if (args.HasArgs(2))
        {
            message = args.GetStringArgument(1);
            optionalMessage = message.c_str();
        }
        msg.WriteString(optionalMessage);

        NetSession::instance->SendMessageDirect(to, msg);
    }
    else
    {
        Console::instance->PrintLine("NetSession hasn't been started yet. Please run NetSessionStart first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(setpacketlatency)
{
    if (!(args.HasArgs(1) || args.HasArgs(2)))
    {
        Console::instance->PrintLine("setpacketloss <latency lower bound (ms)> [latency upper bound (ms)]", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        float latencyLowerBound = args.GetFloatArgument(0);
        float latencyUpperBound = latencyLowerBound;
        if (args.HasArgs(2))
        {
            latencyUpperBound = args.GetFloatArgument(1);
        }
        NetSession::instance->m_packetChannel.m_additionalLagMilliseconds = Range<double>(latencyLowerBound, latencyUpperBound);
    }
    else
    {
        Console::instance->PrintLine("NetSession isn't running. Please run NetSessionStart first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(setpacketloss)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("setpacketloss <float lossPercentage>", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        float lossPercentage = args.GetFloatArgument(0);
        if (lossPercentage < 0.0f || lossPercentage > 1.0f)
        {
            Console::instance->PrintLine("Please enter a percentage between 0 and 1.", RGBA::RED);
            return;
        }
        NetSession::instance->m_packetChannel.m_dropRate = lossPercentage;
    }
    else
    {
        Console::instance->PrintLine("NetSession isn't running. Please run NetSessionStart first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nscreateconn)
{
    if (!(args.HasArgs(3)))
    {
        Console::instance->PrintLine("nscreateconn <index> <guid> <address>", RGBA::RED);
        return;
    }
    if (nullptr != NetSession::instance)
    {
        sockaddr_in address;
        std::string guid = args.GetStringArgument(1);
        std::string fullIPString = args.GetStringArgument(2);

        size_t portLocation = fullIPString.find(":");
        if (portLocation == fullIPString.npos)
        {
            address = NetSystem::StringToSockAddrIPv4(fullIPString.c_str(), GAME_PORT);
        }
        else
        {
            std::string ipPart = fullIPString.substr(0, portLocation);
            std::string portPart = fullIPString.substr(portLocation + 1, fullIPString.length() - portLocation);
            address = NetSystem::StringToSockAddrIPv4(ipPart.c_str(), (uint16_t)std::stoi(portPart));
        }

        NetSession::instance->CreateConnection((uint8_t)args.GetIntArgument(0), guid.c_str(), address);
        Console::instance->PrintLine(Stringf("Created connection at index %i", args.GetIntArgument(0)), RGBA::ORANGE);
    }
    else
    {
        Console::instance->PrintLine("NetSession isn't running. Please run NetSessionStart first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nsdestroyconn)
{
    if (!(args.HasArgs(1)))
    {
        Console::instance->PrintLine("nscreateconn <index>", RGBA::RED);
        return;
    }
    if (nullptr != NetSession::instance)
    {
        NetSession::instance->Disconnect((uint8_t)args.GetIntArgument(0));
        Console::instance->PrintLine(Stringf("Destroyed connection at index %i", args.GetIntArgument(0)), RGBA::ORANGE);        
    }
    else
    {
        Console::instance->PrintLine("NetSession isn't running. Please run NetSessionStart first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nsdebug)
{
    UNUSED(args);
    if (!NetSession::instance)
    {
        Console::instance->PrintLine("Error: NetSession not initialized. Run nsinit.", RGBA::SEA_GREEN);
        return;
    }
    NetSession::instance->m_sessionInfoText = Console::instance->PrintDynamicLine("Session bound to [null]", RGBA::CHOCOLATE);
    NetSession::instance->m_netLagText = Console::instance->PrintDynamicLine("Simulated Net Lag: null", RGBA::CHOCOLATE);
    NetSession::instance->m_netLossText = Console::instance->PrintDynamicLine("Simulated Net Loss: null", RGBA::CHOCOLATE);
    NetSession::instance->m_connectionCountText = Console::instance->PrintDynamicLine("Connection Count: null", RGBA::CHOCOLATE);
    NetSession::instance->m_connectionsText.clear();
    for (unsigned int i = 0; i < NetSession::MAX_CONNECTIONS; ++i)
    {
        NetSession::instance->m_connectionsText.push_back(Console::instance->PrintDynamicLine(Stringf("  [%i] No Connection", i), RGBA::CHOCOLATE));
    }
    Console::instance->m_consoleUpdate.RegisterMethod(NetSession::instance, &NetSession::UpdateNetDebug);
    Console::instance->m_consoleClear.RegisterMethod(NetSession::instance, &NetSession::ShutdownNetDebug);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(toggletimeout)
{
    UNUSED(args);
    NetSession::instance->m_timeoutEnabled = !NetSession::instance->m_timeoutEnabled;
    const char* enabledText = NetSession::instance->m_timeoutEnabled ? "Enabled" : "Disabled";
    Console::instance->PrintLine(Stringf("Net Session timeout %s.", enabledText));
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(hb)
{
    if (!(args.HasArgs(0) || args.HasArgs(1)))
    {
        Console::instance->PrintLine("hb <ip address>", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        // so it's expecting a full address string,
        // ex: 192.168.52.127:4334
        sockaddr_in to;
        if (args.HasArgs(0))
        {
            to = *(sockaddr_in*)NetSystem::GetLocalHostAddressUDP(GAME_PORT_STR);
        }
        else
        {
            std::string fullIPString = args.GetStringArgument(0);
            size_t portLocation = fullIPString.find(":");
            if (portLocation == fullIPString.npos)
            {
                to = NetSystem::StringToSockAddrIPv4(fullIPString.c_str(), GAME_PORT);
            }
            else
            {
                std::string ipPart = fullIPString.substr(0, portLocation);
                std::string portPart = fullIPString.substr(portLocation + 1, fullIPString.length() - portLocation);
                to = NetSystem::StringToSockAddrIPv4(ipPart.c_str(), (uint16_t)std::stoi(portPart));
            }
        }

        NetMessage msg(NetMessage::HEARTBEAT);
        NetConnection* conn = NetSession::instance->GetConnection(NetSession::instance->GetConnectionIndexFromAddress(to));
        conn->SendMessage(msg);
    }
    else
    {
        Console::instance->PrintLine("NetSession hasn't been initialized yet. Please run nsinit first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(iohb)
{
    if (!(args.HasArgs(0) || args.HasArgs(1)))
    {
        Console::instance->PrintLine("iohb <ip address>", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        // so it's expecting a full address string,
        // ex: 192.168.52.127:4334
        sockaddr_in to;
        if (args.HasArgs(0))
        {
            to = *(sockaddr_in*)NetSystem::GetLocalHostAddressUDP(GAME_PORT_STR);
        }
        else
        {
            std::string fullIPString = args.GetStringArgument(0);
            size_t portLocation = fullIPString.find(":");
            if (portLocation == fullIPString.npos)
            {
                to = NetSystem::StringToSockAddrIPv4(fullIPString.c_str(), GAME_PORT);
            }
            else
            {
                std::string ipPart = fullIPString.substr(0, portLocation);
                std::string portPart = fullIPString.substr(portLocation + 1, fullIPString.length() - portLocation);
                to = NetSystem::StringToSockAddrIPv4(ipPart.c_str(), (uint16_t)std::stoi(portPart));
            }
        }

        NetMessage msg(NetMessage::INORDER_HEARTBEAT);
        NetConnection* conn = NetSession::instance->GetConnection(NetSession::instance->GetConnectionIndexFromAddress(to));
        conn->SendMessage(msg);
    }
    else
    {
        Console::instance->PrintLine("NetSession hasn't been initialized yet. Please run nsinit first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(nethost)
{
    if (!(args.HasArgs(0) || args.HasArgs(1)))
    {
        Console::instance->PrintLine("nethost <username>", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        if (NetSession::instance->m_hostConnection != nullptr)
        {
            Console::instance->PrintLine("Already have a host connection", RGBA::SADDLE_BROWN);
            return;
        }
        if (NetSession::instance->GetSessionState() != NetSession::DISCONNECTED)
        {
            Console::instance->PrintLine("In an invalid state, must be disconnected to run this command", RGBA::SADDLE_BROWN);
            return;
        }
        std::string username;
        if (args.HasArgs(0))
        {
            username = NetSystem::GetLocalHostName();
        }
        else
        {
            username = args.GetStringArgument(0);
        }
        NetSession::instance->Host(username.c_str());
        Console::instance->PrintLine("Began hosting.", RGBA::SADDLE_BROWN);
    }
    else
    {
        Console::instance->PrintLine("NetSession hasn't been initialized yet. Please run nsinit first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(netleave)
{
    UNUSED(args)
    if (NetSession::instance != nullptr)
    {
        if (NetSession::instance->GetSessionState() != NetSession::CONNECTED)
        {
            Console::instance->PrintLine("Not currently connected", RGBA::KHAKI);
            return;
        }
        NetSession::instance->Leave();
        Console::instance->PrintLine("Left the session.", RGBA::KHAKI);
    }
    else
    {
        Console::instance->PrintLine("NetSession hasn't been initialized yet. Please run nsinit first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(netjoin)
{
    if (!(args.HasArgs(2)))
    {
        Console::instance->PrintLine("netjoin <username> <hostaddr : hostport>", RGBA::RED);
        return;
    }
    if (NetSession::instance != nullptr)
    {
        if (NetSession::instance->m_hostConnection != nullptr)
        {
            Console::instance->PrintLine("Already have a host connection", RGBA::SADDLE_BROWN);
            return;
        }
        if (NetSession::instance->GetSessionState() != NetSession::DISCONNECTED)
        {
            Console::instance->PrintLine("In an invalid state, must be disconnected to run this command", RGBA::SADDLE_BROWN);
            return;
        }
        sockaddr_in address;
        std::string guid = args.GetStringArgument(0);
        std::string fullIPString = args.GetStringArgument(1);

        size_t portLocation = fullIPString.find(":");
        if (portLocation == fullIPString.npos)
        {
            address = NetSystem::StringToSockAddrIPv4(fullIPString.c_str(), GAME_PORT);
        }
        else
        {
            std::string ipPart = fullIPString.substr(0, portLocation);
            std::string portPart = fullIPString.substr(portLocation + 1, fullIPString.length() - portLocation);
            address = NetSystem::StringToSockAddrIPv4(ipPart.c_str(), (uint16_t)std::stoi(portPart));
        }

        NetSession::instance->Join(guid.c_str(), address);
        Console::instance->PrintLine("Submitted request to join.", RGBA::SADDLE_BROWN);
    }
    else
    {
        Console::instance->PrintLine("NetSession hasn't been initialized yet. Please run nsinit first.", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(go)
{
    UNUSED(args)
    Console::instance->RunCommand("csjoin");
    //Console::instance->RunCommand("bcmd nscreateconn 0 right 192.168.0.4:4334");
    //Console::instance->RunCommand("bcmd nscreateconn 1 left 192.168.0.4:4335");    
    Console::instance->RunCommand("bcmd nscreateconn 0 right 10.8.151.65:4334");
    Console::instance->RunCommand("bcmd nscreateconn 1 left 10.8.151.65:4335");
    Console::instance->RunCommand("nsdebug");
    Console::instance->RunCommand("rcmd nsdebug");
}