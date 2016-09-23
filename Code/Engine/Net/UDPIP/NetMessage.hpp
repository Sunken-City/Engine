#pragma once
#include "Engine/DataStructures/BytePacker.hpp"
#include <stdint.h>

typedef unsigned char byte;
struct NetSender; 
struct NetMessageDefinition;

#define MESSAGE_MTU 1024
#define BIT_FLAG(f) (1 << (f))

class NetMessage : public BytePacker
{
public:
    //ENUMS/////////////////////////////////////////////////////////////////////
    enum CoreMessageTypes : byte
    {
        PING = 0,
        PONG = 1,
        HEARTBEAT,
        INORDER_HEARTBEAT,
        ACK,
        JOIN_REQUEST,
        JOIN_DENY,
        JOIN_ACCEPT,
        CONNECTION_LEAVE,
        KICK,
        QUIT,
        NUM_MESSAGES

    };

    //------------------------------------------------------------------------
    // Used to define how the message is transferred.
    // By defualt all messages are unreliable and unordered (UDP)
    enum class Option : uint8_t
    {
        NONE = 0,
        UNRELIABLE = 0,
        //Message is guaranteed to arrive
        RELIABLE = BIT_FLAG(0),
        // Message will be processed in order sent
        INORDER = BIT_FLAG(1),
    };

    //------------------------------------------------------------------------
    // Used to dictate the conditions in which a message CAN be processed.
    // Usually used for valdation to make sure messages are used as intended.
    enum class Control : uint8_t
    {
        NONE = 0,
        // By default, messages are ignored if they don't have a connect
        // unless they have this flag.
        PROCESS_CONNECTIONLESS = BIT_FLAG(0),
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NetMessage()
        : BytePacker(m_msgBuffer, MESSAGE_MTU, 0, IBinaryReader::BIG_ENDIAN) 
        , m_reliableId(0)
        , m_sequenceId(0)
        , m_lastSentTimestampMs(0)
    {
    };

    NetMessage(uint8_t msg_idx)
        : BytePacker(m_msgBuffer, MESSAGE_MTU, 0, IBinaryReader::BIG_ENDIAN)
        , m_type(msg_idx)
        , m_reliableId(0)
        , m_sequenceId(0)
        , m_lastSentTimestampMs(0)
    {
    };

    NetMessage(const NetMessage& other)
        : BytePacker(m_msgBuffer, MESSAGE_MTU, other.GetReadableBytes(), IBinaryReader::BIG_ENDIAN)
    {
        m_type = other.m_type;
        m_reliableId = other.m_reliableId;
        m_sequenceId = other.m_sequenceId;
        m_lastSentTimestampMs = other.m_lastSentTimestampMs;
        memcpy(m_msgBuffer, other.m_msgBuffer, other.GetReadableBytes());
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    size_t GetHeaderSize() const;
    size_t GetPayloadSize() const;
    void Process(const NetSender& from);
    bool IsReliable() const;
    const NetMessageDefinition* GetDefinition() const;
    bool RequiresConnection() const;
    bool IsInOrder();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    byte m_type;
    // Reliable ID assigned
    uint16_t m_reliableId;
    uint16_t m_sequenceId;
    // If reliable, time since this was last attempted to be sent
    uint32_t m_lastSentTimestampMs;
    byte m_msgBuffer[MESSAGE_MTU];
};