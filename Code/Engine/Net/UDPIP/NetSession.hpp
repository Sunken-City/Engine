#pragma once
#include "Engine/Net/UDPIP/PacketChannel.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Core/Event.hpp"

#define GAME_PORT_STR "4334"
#define GAME_PORT 4334
#define PACKET_MTU 1232
#define MESSAGE_MTU 1024

class NetSession;
class NetConnection;
struct ColoredText;

//-----------------------------------------------------------------------------------
struct NetSender
{
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NetSender()
        : session(nullptr)
        , connection(nullptr)
    {};

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    sockaddr_in address;
    NetSession* session;
    NetConnection* connection;
};

typedef void(NetMessageCallback)(const NetSender&, NetMessage&);

//-----------------------------------------------------------------------------------
struct NetMessageDefinition
{
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NetMessageDefinition()
        : id(0)
        , debugName(nullptr)
        , callbackFunction(nullptr)
        , m_controlFlags(0)
        , m_optionFlags(0)
    {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetOptionFlag(NetMessage::Option flag);
    void SetControlFlag(NetMessage::Control flag);
    bool HasOptionFlag(NetMessage::Option flag) const;
    bool HasControlFlag(NetMessage::Control controlFlag) const;
    void ClearOptionFlag(NetMessage::Option flag);
    void ClearControlFlag(NetMessage::Control flag);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    uint8_t id;
    uint32_t m_controlFlags;
    uint32_t m_optionFlags;
    const char* debugName;
    NetMessageCallback* callbackFunction;
};

//-----------------------------------------------------------------------------------
class NetSession
{
public:
    //ENUMS/////////////////////////////////////////////////////////////////////
    enum State
    {
        INVALID,
        DISCONNECTED,
        HOSTING,
        JOINING,
        CONNECTED,
        NUM_STATES
    };

    enum ErrorCode : uint8_t
    {
        NONE,
        START_FAILED_TO_CREATE_SOCKET,
        JOIN_ERROR_HOST_TIMEOUT,
        JOIN_DENIED_NOT_ACCEPTING_NEW_CONNECTIONS,
        JOIN_DENIED_NOT_HOST,
        JOIN_DENIED_FULL,
        JOIN_DENIED_GUID_IN_USE,
        ERROR_HOST_DISCONNECTED,
        NUM_CODES
    };

    //CONSTRUCTOS/////////////////////////////////////////////////////////////////////
    NetSession();
    ~NetSession();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    bool Start(const char* portNumber);
    void Stop();
    void Cleanup();
    void Update(float deltaSeconds);
    void UpdateNetDebug();
    void ShutdownNetDebug();

    void Host(const char* username);
    void Join(const char* username, sockaddr_in& hostAddress);
    void Leave();

    void ProcessIncomingPackets(const size_t maxPacketsToProcess = INFINITY);
    void ProcessIncomingPacket(NetSender& from, NetPacket& packet);
    void RegisterMessage(uint8_t type, const char* messageName, NetMessageCallback* functionPointer, uint32_t optionFlags, uint32_t controlFlags);
    void SendMessageDirect(const sockaddr_in& to, const NetMessage& msg);
    size_t SendMessagesDirect(sockaddr_in& to, NetMessage** messages, size_t numMessages);
    bool Connect(NetConnection* cp, const uint8_t idx);
    void Disconnect(NetConnection* cp);
    void Disconnect(uint8_t index);
    void CheckForTimeouts();
    void CheckForJoinResponse();
    NetConnection* CreateConnection(uint8_t index, const char* guid, sockaddr_in address);
    void DestroyConnection(uint8_t index);

    //QUERIES/////////////////////////////////////////////////////////////////////
    NetConnection* GetConnection(uint8_t index);
    inline NetConnection* GetMyConnection() { return m_myConnection; };
    inline NetConnection* GetHostConnection() { return m_hostConnection; };
    uint8_t GetMyConnectionIndex();
    uint8_t GetConnectionIndexFromAddress(const sockaddr_in& address);
    void SendDeny(ErrorCode reason, const sockaddr_in& address);
    const NetMessageDefinition* FindDefinition(byte messageType);
    bool CanProcessMessage(const NetSender& from, const NetMessage& msg) const;
    bool IsRunning();
    bool AmIConnected();
    static const char* GetErrorCodeCstr(const ErrorCode& code);
    bool IsPartyFull();
    bool IsGuidInUse(const char* guid);
    uint8_t GetNextAvailableIndex();
    sockaddr_in GetAddress() { return m_packetChannel.GetAddress(); };

    //STATE MANAGEMENT/////////////////////////////////////////////////////////////////////
    bool SetSessionState(State newState, void(NetSession::*onStateSwitchCallback)() = nullptr);
    inline State GetSessionState() { return m_sessionState; };
    const char* GetStateCstr(const State& state);
    void OnEnterDisconnectedState();

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static const uint8_t MAX_CONNECTIONS = 8;
    static const uint8_t INVALID_CONNECTION_INDEX = 255;
    static const int MAX_DEFINITIONS = 256;

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static NetSession* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    PacketChannel m_packetChannel;
    NetMessageDefinition m_netMessageDefinitions[MAX_DEFINITIONS]; //container of definitions
    NetConnection* m_allConnections[MAX_CONNECTIONS];
    NetConnection* m_myConnection;
    NetConnection* m_hostConnection;
    Event<NetConnection*> m_OnConnectionJoin;
    Event<NetConnection*> m_OnConnectionLeave;
    Event<NetConnection*> m_OnNetTick;
    Event<> m_OnStateSwitch; //ONE SHOT FUNCTIONS ONLY. These get registered when switching to the state, and then are fired when swapping away from that state and removed.
    float m_timeSinceLastUpdate;
    double m_timeLastJoinRequestSent;
    float m_tickRate;
    unsigned int m_numConnections;
    State m_sessionState;
    ErrorCode m_lastError;
    bool m_timeoutEnabled;
    bool m_isListening;

    //NetDebug console line references, used for making a dynamic updates in my god-awful console.
    ColoredText* m_sessionInfoText;
    ColoredText* m_netLagText;
    ColoredText* m_netLossText;
    ColoredText* m_connectionCountText;
    std::vector<ColoredText*> m_connectionsText;
};