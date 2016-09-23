#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"

//-----------------------------------------------------------------------------------
size_t NetMessage::GetHeaderSize() const
{
    return sizeof(uint8_t) + sizeof(uint16_t);
}

//-----------------------------------------------------------------------------------
size_t NetMessage::GetPayloadSize() const
{
    return GetReadableBytes();
}

//-----------------------------------------------------------------------------------
void NetMessage::Process(const NetSender& from)
{
    const NetMessageDefinition* messageDef = NetSession::instance->FindDefinition(m_type);
    if (messageDef != nullptr)
    {
        messageDef->callbackFunction(from, *this);
    }
}

//-----------------------------------------------------------------------------------
bool NetMessage::IsReliable() const
{
    return GetDefinition()->HasOptionFlag(Option::RELIABLE);
}

//-----------------------------------------------------------------------------------
const NetMessageDefinition* NetMessage::GetDefinition() const
{
    return NetSession::instance->FindDefinition(m_type);
}

//-----------------------------------------------------------------------------------
bool NetMessage::RequiresConnection() const
{
    return !GetDefinition()->HasControlFlag(NetMessage::Control::PROCESS_CONNECTIONLESS);
}

//-----------------------------------------------------------------------------------
bool NetMessage::IsInOrder()
{
    return GetDefinition()->HasOptionFlag(Option::INORDER);
}
