#pragma once
#pragma comment(lib, "ws2_32")
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdint.h>
#include <vector>
#include <queue>
#include <set>

class NetSession;
class NetMessage;
class NetPacket;
struct NetSender;

class NetConnection
{
public:

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static const int MAX_GUID_LENGTH = 32;
    static const int MAX_ACK_BUNDLES = 64;
    static const int TIMEOUT_TIME_MS = 15000;
    static const int BAD_CONNECTION_TIME_MS = 5000;
    static const int MAX_RELIABLES_PER_PACKET = 32;
    static const uint16_t MAX_RELIABLE_RANGE = 1000;
    static const uint16_t INVALID_PACKET_ACK = 0xFFFF;

    //ENUMS/////////////////////////////////////////////////////////////////////
    enum State
    {
        BAD,
        LOCAL,
        UNCONFIRMED,
        CONFIRMED,
        NUM_STATES
    };

    //STRUCT BUNDLE/////////////////////////////////////////////////////////////////////
    // Ack Bundles are ways to attach information to a given outgoing packet
    // so upon that ack being confirmed, we can do some cleanup
    struct AckBundle
    {
        void AddReliable(uint16_t reliableId);

        uint16_t ack;
        uint32_t reliableCount;
        // What reliables were sent with this ack?
        std::vector<uint16_t> sentReliableIds;
    };

    struct Info
    {
        sockaddr_in address;
        char m_guid[MAX_GUID_LENGTH];
        uint8_t index;
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NetConnection(uint8_t index, const char* guid, const sockaddr_in& address, NetSession* session);
    ~NetConnection();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SendMessage(NetMessage& msg);
    void ConstructAndSendPacket();
    uint8_t AttachOldReliables(NetPacket& p, AckBundle* ackBundle);
    uint8_t AttachUnsentReliables(NetPacket& p, AckBundle* ab);
    uint8_t AttachUnreliables(NetPacket& p);
    void UpdateHighestValue(uint16_t newValue);
    void MarkPacketReceived(const NetPacket& packet);
    void ConfirmAck(uint16_t ack);
    bool IsHostConnection();
    bool IsMyConnection();
    inline bool IsConnected() { return m_state == CONFIRMED || m_state == LOCAL || m_state == BAD; };
    const char* GetStateCstr();
    void MarkMessageReceived(const NetMessage& msg);
    void ProcessMessage(const NetSender& from, NetMessage& msg); //Called if we can process a message and will mark the message as recieved
    void ProcessInOrder(const NetSender& from, NetMessage& msg);
    void AddInOrderOfSequenceId(NetMessage* msg);
    bool IsOld(NetMessage* msg);
    AckBundle* CreateBundle(uint16_t ack);
    bool CanProcessMessage(const NetMessage& msg); // should we process this message (checks controls and records) such as it already being received
    uint16_t GetLastSentAck() { return m_nextSentAck - 1; };
    uint16_t GetMostRecentConfirmedAck() { return m_highestReceivedAck; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    //Identifying info
    uint8_t m_index;
    sockaddr_in m_address;
    char m_guid[MAX_GUID_LENGTH];
    NetSession* m_session;

    //State info
    State m_state;
    double m_lastSentTimeMs;
    double m_lastRecievedTimeMs;

private:
    //PRIVATE FUNCTIONS/////////////////////////////////////////////////////////////////////
    //Send side:  reliable traffic
    bool CanAttachNewReliable(); // determines if we can send a new reliable message (have reliables IDs to spare)
    size_t GetLiveReliableRange() { return m_lastSentReliableId - m_oldestUnconfirmedReliableId; }; // distance between last_sent_reliable_id, and oldest_unconfirmed_reliable_id
    uint16_t GetNextReliableID();

    // Mark a reliable ID as confirmed - that is, we know the other guy has processed it
    void MarkReliableConfirmed(const uint16_t reliableId);
    bool IsReliableConfirmed(const uint16_t reliableId);
    bool CycleGreaterThanEqual(uint16_t a, uint16_t b);
    AckBundle* FindBundle(uint16_t ack);

    // recv_side: reliable traffic
    bool HasReceivedReliable(const uint16_t reliableId);	// check if a reliable_id is marked as received
    void MarkReliableReceived(const uint16_t reliableId); 	// after processing a message, mark it as received
    void RemoveAllReliableIdsLessThan(const uint16_t value);

    //PRIVATE MEMBERS/////////////////////////////////////////////////////////////////////
    //Acks
    //sending
    uint16_t m_nextSentAck;
    AckBundle m_ackBundles[MAX_ACK_BUNDLES];
    //recieving
    uint16_t m_nextExpectedAck; // should always be highest_received_ack + 1.
    uint16_t m_highestReceivedAck; // so there's no real need for both
    uint16_t m_previousHighestReceivedAcksBitfield; // bitfield of previous received acks

    //Messages
    std::vector<NetMessage*> m_unreliables;
    std::queue<NetMessage*> m_unsentReliables;
    std::queue<NetMessage*> m_sentReliables;

    //Sending reliable traffic
    uint16_t m_nextSentReliableId;
    uint16_t m_lastSentReliableId;
    uint16_t m_oldestUnconfirmedReliableId;
    std::set<uint16_t> m_confirmedReliableIds;

    //Recieving reliable traffic
    uint16_t m_nextExpectedReliableId;
    std::vector<uint16_t> m_receivedReliableIds;

    //Sending InOrder
    uint16_t m_nextSentSequenceId;

    //Receiving InOrder
    uint16_t m_nextExpectedReceivedSequenceId;
    std::vector<NetMessage*> m_outOfOrderReceivedSequencedMessages;
};