#pragma once
#include "Engine/DataStructures/BytePacker.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#pragma comment(lib, "ws2_32")
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define PACKET_MTU 1232

class NetPacket : public BytePacker
{
public:
    //STRUCTS/////////////////////////////////////////////////////////////////////
    struct Header
    {
        //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
        Header() : fromConnectionIndex(INVALID_CONNECTION_INDEX), messageCount(0) {};
        Header(uint8_t connectionIndex = INVALID_CONNECTION_INDEX) : fromConnectionIndex(connectionIndex), messageCount(0) {};

        //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
        uint8_t fromConnectionIndex;
        uint16_t ack;
        uint16_t highestReceivedAck;
        uint16_t previousReceivedAcksBitfield;
        uint8_t messageCount;
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NetPacket(uint8_t connectionIndex = 0)
        : BytePacker(m_buffer, PACKET_MTU, 0, IBinaryReader::BIG_ENDIAN)
        , m_header(connectionIndex)
    {

    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void WriteHeader();
    void ReadHeader();
    size_t WriteMessage(NetMessage* msg);
    size_t WriteMessages(NetMessage** messages, size_t count);
    void WriteMessageAndFinalize(const NetMessage& message);
    size_t WriteMessagesAndFinalize(NetMessage** messages, size_t count);
    void ReadMessage(NetMessage& outMessage);
    NetMessage ReadMessage();
    bool CanWrite(NetMessage* msg);
    uint8_t* GetMessageCountBookmark();

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static const uint8_t INVALID_CONNECTION_INDEX = 255;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    byte m_buffer[PACKET_MTU];
    Header m_header;
    sockaddr_in m_fromAddress;

private:
    uint8_t* m_msgCountBookmark;
};