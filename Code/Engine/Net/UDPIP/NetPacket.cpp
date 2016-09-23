#include "Engine/Net/UDPIP/NetPacket.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"
#include "Engine/Input/Logging.hpp"

//-----------------------------------------------------------------------------------
void NetPacket::WriteHeader()
{
    Write<uint8_t>(m_header.fromConnectionIndex);
    m_msgCountBookmark = Reserve<uint8_t>(m_header.messageCount);
    Write<uint16_t>(m_header.ack);
    Write<uint16_t>(m_header.highestReceivedAck);
    Write<uint16_t>(m_header.previousReceivedAcksBitfield);
}

//-----------------------------------------------------------------------------------
void NetPacket::ReadHeader()
{
    Read<uint8_t>(m_header.fromConnectionIndex);
    Read<uint8_t>(m_header.messageCount);
    Read<uint16_t>(m_header.ack);
    Read<uint16_t>(m_header.highestReceivedAck);
    Read<uint16_t>(m_header.previousReceivedAcksBitfield);
}

//-----------------------------------------------------------------------------------
bool NetPacket::CanWrite(NetMessage* msg)
{
    return GetWritableBytes() >= msg->GetHeaderSize() + msg->GetPayloadSize() + sizeof(uint16_t);
}

//-----------------------------------------------------------------------------------
uint8_t* NetPacket::GetMessageCountBookmark()
{
    return m_msgCountBookmark;
}

//-----------------------------------------------------------------------------------
size_t NetPacket::WriteMessage(NetMessage* msg)
{
#pragma todo("Replace the other redundant functions once you're not on fire")

    const NetMessageDefinition* definition = NetSession::instance->FindDefinition(msg->m_type);

    size_t messageSize = msg->GetHeaderSize() + msg->GetPayloadSize();
    size_t total = messageSize + sizeof(uint16_t);
    if (GetWritableBytes() >= total)
    {
        //Can write!
        Write<uint16_t>((const uint16_t)messageSize);
        Write<uint8_t>((const uint8_t)definition->id);
        Write<uint16_t>(msg->m_reliableId);
        Write<uint16_t>(msg->m_sequenceId);
        WriteBytes(msg->m_buffer, msg->GetPayloadSize());
        return total;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------------------------------------------------------
size_t NetPacket::WriteMessages(NetMessage** messages, size_t count)
{
    size_t numMessagesCanFit;
    size_t totalBytes = 0;
    for (numMessagesCanFit = 0; numMessagesCanFit < count; ++numMessagesCanFit)
    {
        size_t messageSize = sizeof(uint16_t) + messages[numMessagesCanFit]->GetHeaderSize() + messages[numMessagesCanFit]->GetPayloadSize();
        if (GetWritableBytes() < totalBytes + messageSize)
        {
            break;
        }
        totalBytes += messageSize;
    }

    for (size_t i = 0; i < numMessagesCanFit; ++i)
    {
        NetMessage* message = messages[i];
        const NetMessageDefinition* definition = NetSession::instance->FindDefinition(message->m_type);
        Write<uint16_t>((const uint16_t)(message->GetHeaderSize() + message->GetPayloadSize()));
        Write<uint8_t>(definition->id);
        Write<uint16_t>(message->m_reliableId);
        Write<uint16_t>(message->m_sequenceId);
        WriteBytes(message->m_buffer, message->GetPayloadSize());
    }
    return numMessagesCanFit;
}

//-----------------------------------------------------------------------------------
void NetPacket::WriteMessageAndFinalize(const NetMessage& message)
{
    //Writing a single message.
    m_header.messageCount = 1;
    WriteHeader();

    const NetMessageDefinition* definition = NetSession::instance->FindDefinition(message.m_type);

    size_t messageSize = message.GetHeaderSize() + message.GetPayloadSize();
    size_t total = messageSize + sizeof(uint16_t);
    if (GetWritableBytes() >= total)
    {
        //Can write!
        Write<uint16_t>((const uint16_t)messageSize);
        Write<uint8_t>((const uint8_t)definition->id);
        Write<uint16_t>(message.m_reliableId);
        Write<uint16_t>(message.m_sequenceId);
        WriteBytes(message.m_buffer, message.GetPayloadSize());
    }
}

//-----------------------------------------------------------------------------------
size_t NetPacket::WriteMessagesAndFinalize(NetMessage** messages, size_t count)
{
    size_t numMessagesCanFit;
    size_t totalBytes = 0;
    for (numMessagesCanFit = 0; numMessagesCanFit < count; ++numMessagesCanFit)
    {
        size_t messageSize = sizeof(uint16_t) + messages[numMessagesCanFit]->GetHeaderSize() + messages[numMessagesCanFit]->GetPayloadSize();
        if (GetWritableBytes() < totalBytes + messageSize)
        {
            break;
        }
        totalBytes += messageSize;
    }
    m_header.messageCount = (const uint8_t)numMessagesCanFit;
    WriteHeader();
    for (size_t i = 0; i < numMessagesCanFit; ++i)
    {
        NetMessage* message = messages[i];
        const NetMessageDefinition* definition = NetSession::instance->FindDefinition(message->m_type);
        Write<uint16_t>((const uint16_t)(message->GetHeaderSize() + message->GetPayloadSize()));
        Write<uint8_t>(definition->id);
        Write<uint16_t>(message->m_reliableId);
        Write<uint16_t>(message->m_sequenceId);
        WriteBytes(message->m_buffer, message->GetPayloadSize());
    }
    return numMessagesCanFit;
}

//-----------------------------------------------------------------------------------
NetMessage NetPacket::ReadMessage()
{
    NetMessage msg;
    ReadMessage(msg);
    return msg;
}

//-----------------------------------------------------------------------------------
void NetPacket::ReadMessage(NetMessage& outMessage)
{
    //Read message size and verify
    uint16_t msgSize;
    Read<uint16_t>(msgSize);
    if (msgSize > (MESSAGE_MTU))
    {
        LogPrintf(LogLevel::WARNING, "Invalid Packet thrown out.");
        return;
    }

    //Read message contents
    outMessage.SetReadableBytes(msgSize - outMessage.GetHeaderSize());
    Read<uint8_t>(outMessage.m_type);
    Read<uint16_t>(outMessage.m_reliableId);
    Read<uint16_t>(outMessage.m_sequenceId);
    ReadBytes(outMessage.m_msgBuffer, msgSize - outMessage.GetHeaderSize());
}


