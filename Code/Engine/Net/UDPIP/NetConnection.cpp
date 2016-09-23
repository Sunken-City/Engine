#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Time/Time.hpp"

//-----------------------------------------------------------------------------------
NetConnection::NetConnection(uint8_t index, const char* guid, const sockaddr_in& address, NetSession* session)
    : m_index(index)
    , m_address(address)
    , m_session(session)
    , m_state(State::UNCONFIRMED)
    , m_lastSentTimeMs(GetCurrentTimeMilliseconds())
    , m_lastRecievedTimeMs(GetCurrentTimeMilliseconds())
    , m_previousHighestReceivedAcksBitfield(0)
    , m_highestReceivedAck(INVALID_PACKET_ACK)
    , m_nextSentAck(0)
    , m_nextSentReliableId(0)
    , m_oldestUnconfirmedReliableId(0)
    , m_nextExpectedReliableId(0)
    , m_lastSentReliableId(0)
    , m_nextSentSequenceId(13)
    , m_nextExpectedReceivedSequenceId(13)
{
    memcpy(m_guid, guid, MAX_GUID_LENGTH);
}

//-----------------------------------------------------------------------------------
NetConnection::~NetConnection()
{
    for (NetMessage* msg : m_unreliables)
    {
        delete msg;
    }
    while(!m_unsentReliables.empty())
    {
        NetMessage* msg = m_unsentReliables.front();
        delete msg;
        m_unsentReliables.pop();
    }
    while (!m_sentReliables.empty())
    {
        NetMessage* msg = m_sentReliables.front();
        delete msg;
        m_sentReliables.pop();
    }
    for (NetMessage* msg : m_outOfOrderReceivedSequencedMessages)
    {
        delete msg;
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::SendMessage(NetMessage& msg)
{
    NetMessage* nextMsg = new NetMessage(msg);
    if (nextMsg->IsInOrder())
    {
        nextMsg->m_sequenceId = m_nextSentSequenceId;
        m_nextSentSequenceId++;
    }
    if (nextMsg->IsReliable()) 
    {
        m_unsentReliables.push(nextMsg);
    }
    else 
    {
        m_unreliables.push_back(nextMsg);
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::ConstructAndSendPacket()
{
    //Initialize the packet
    NetPacket packet(m_session->GetMyConnectionIndex());
    packet.m_header.ack = m_nextSentAck++;
    packet.m_header.highestReceivedAck = m_highestReceivedAck;
    packet.m_header.previousReceivedAcksBitfield = m_previousHighestReceivedAcksBitfield;

    //Prepare the packet body
    packet.WriteHeader();
    // Reserve space
    uint8_t* msgsWritten = packet.GetMessageCountBookmark();

    AckBundle* bundle = CreateBundle(packet.m_header.ack);

    uint8_t sent = 0;
    sent += AttachOldReliables(packet, bundle);

    sent += AttachUnsentReliables(packet, bundle);

    sent += AttachUnreliables(packet);
    for (NetMessage* msg : m_unreliables)
    {
        delete msg;
    }
    m_unreliables.clear();

    *msgsWritten = sent;
    m_lastSentTimeMs = GetCurrentTimeMilliseconds();

    m_session->m_packetChannel.SendTo(m_address, packet.m_buffer, packet.GetTotalReadableBytes());
}

//-----------------------------------------------------------------------------------
uint8_t NetConnection::AttachOldReliables(NetPacket& p, AckBundle* ackBundle)
{
    uint8_t numMessagesAdded = 0;
    while (!m_sentReliables.empty())
    {
        NetMessage* msg = m_sentReliables.front();
        if (IsReliableConfirmed(msg->m_reliableId))
        {
            m_sentReliables.pop();
            delete msg;
            continue;
        }
        if (IsOld(msg) && p.CanWrite(msg))
        {
            m_sentReliables.pop();
            msg->m_lastSentTimestampMs = (uint32_t)GetCurrentTimeMilliseconds();
            p.WriteMessage(msg);
            ++numMessagesAdded;
            ackBundle->AddReliable(msg->m_reliableId);
            m_sentReliables.push(msg);
        }
        else
        {
            break;
        }
    }
    return numMessagesAdded;
}

//-----------------------------------------------------------------------------------
uint8_t NetConnection::AttachUnsentReliables(NetPacket& packet, AckBundle* ackBundle)
{
    uint8_t numMessagesAdded = 0;
    while (!m_unsentReliables.empty() && CanAttachNewReliable())
    {
        NetMessage* msg = m_unsentReliables.front();
        if (packet.CanWrite(msg))
        {
            msg->m_reliableId = GetNextReliableID();
            msg->m_lastSentTimestampMs = (uint32_t)GetCurrentTimeMilliseconds();
            packet.WriteMessage(msg);
            ++numMessagesAdded;
            ackBundle->AddReliable(msg->m_reliableId);
            m_unsentReliables.pop();
            m_sentReliables.push(msg);
        }
        else
        {
            break;
        }
    }
    return numMessagesAdded;
}

//-----------------------------------------------------------------------------------
uint8_t NetConnection::AttachUnreliables(NetPacket& p)
{
    return (uint8_t)p.WriteMessages(m_unreliables.data(), m_unreliables.size());
}

//-----------------------------------------------------------------------------------
void NetConnection::UpdateHighestValue(uint16_t newValue)
{
    if (CycleGreaterThanEqual(newValue, m_highestReceivedAck))
    {
        uint16_t shift = newValue - m_highestReceivedAck;
        m_previousHighestReceivedAcksBitfield = m_previousHighestReceivedAcksBitfield << shift;
        m_highestReceivedAck = newValue;
        m_previousHighestReceivedAcksBitfield |= shift - 1;
    }
    else
    {
        uint16_t offset = m_highestReceivedAck - newValue;
        m_previousHighestReceivedAcksBitfield |= offset - 1;
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::MarkPacketReceived(const NetPacket& packet)
{
    //First, update this NetConnection's highest value and previous received acks bitfield (see interview question's AddNewValue).
    //-- > See Class 7's MarkPacketReceived, which puts the below in a subfunction called ProcessConfirmedAcks.
    UpdateHighestValue(packet.m_header.ack);
    ConfirmAck(packet.m_header.highestReceivedAck);
    for (size_t bitIndex = 0; bitIndex < sizeof(packet.m_header.previousReceivedAcksBitfield); ++bitIndex)
    {
        if (((1 << bitIndex) & packet.m_header.previousReceivedAcksBitfield) != 0) //IsBitSetAtIndex( size_t idx, size_t bitfield ) { return ( ( 1<<idx ) & bitfield ) != 0 ; }
        {
            ConfirmAck(packet.m_header.highestReceivedAck - (m_previousHighestReceivedAcksBitfield + 1)); //You're sending in "confirm me as true" for one index of the field?
        }
    }
    m_lastRecievedTimeMs = GetCurrentTimeMilliseconds();
    if (!IsMyConnection())
    {
        m_state = NetConnection::State::CONFIRMED;
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::ConfirmAck(uint16_t ack)
{
    //Find the ack bundle.
    AckBundle* correspondingBundle = FindBundle(ack); //Using the bundles[] on NetConnection.
    if (correspondingBundle != nullptr)
    {
        for each (uint16_t id in correspondingBundle->sentReliableIds)
        {
            MarkReliableConfirmed(id); //confirmedIds.push_back() but more logic.
        }
    }
}

//-----------------------------------------------------------------------------------
bool NetConnection::IsHostConnection()
{
    if (NetSession::instance->m_hostConnection)
    {
        return NetSystem::SockaddrCompare(this->m_address, NetSession::instance->m_hostConnection->m_address);
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------------
bool NetConnection::IsMyConnection()
{
    return NetSystem::SockaddrCompare(this->m_address, NetSession::instance->m_packetChannel.m_socket.m_address);
}

//-----------------------------------------------------------------------------------
const char* NetConnection::GetStateCstr()
{
    switch (m_state)
    {
    case NetConnection::BAD:
        return "Bad";
    case NetConnection::LOCAL:
        return "Local";
    case NetConnection::UNCONFIRMED:
        return "Unconfirmed";
    case NetConnection::CONFIRMED:
        return "Confirmed";
    default:
        ERROR_AND_DIE("Invalid state passed in");
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::MarkMessageReceived(const NetMessage& msg)
{
    if (msg.IsReliable())
    {
        MarkReliableReceived(msg.m_reliableId);
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::ProcessMessage(const NetSender& from, NetMessage& msg)
{
    if (msg.IsInOrder())
    {
        ProcessInOrder(from, msg);
    }
    else
    {
        msg.Process(from); //Call the definition callback on the message with this from
    }
    MarkMessageReceived(msg);
}

//-----------------------------------------------------------------------------------
void NetConnection::ProcessInOrder(const NetSender& from, NetMessage& msg)
{
    if (msg.m_sequenceId == m_nextExpectedReceivedSequenceId)
    {
        msg.Process(from);
        ++m_nextExpectedReceivedSequenceId;
        while (!m_outOfOrderReceivedSequencedMessages.empty() && m_outOfOrderReceivedSequencedMessages[0]->m_sequenceId == m_nextExpectedReceivedSequenceId)
        {
            m_outOfOrderReceivedSequencedMessages[0]->Process(from);
            ++m_nextExpectedReceivedSequenceId;
            delete m_outOfOrderReceivedSequencedMessages[0];
            m_outOfOrderReceivedSequencedMessages.erase(m_outOfOrderReceivedSequencedMessages.begin());
        }
    }
    else
    {
        NetMessage* copy = new NetMessage(msg);
        AddInOrderOfSequenceId(copy);
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::AddInOrderOfSequenceId(NetMessage* newMessage)
{
    for (auto iter = m_outOfOrderReceivedSequencedMessages.begin(); iter != m_outOfOrderReceivedSequencedMessages.end(); ++iter)
    {
        if (newMessage->m_sequenceId < (*iter)->m_sequenceId)
        {
            m_outOfOrderReceivedSequencedMessages.insert(iter, newMessage);
            return;
        }
    }
    m_outOfOrderReceivedSequencedMessages.push_back(newMessage);
}

//-----------------------------------------------------------------------------------
bool NetConnection::IsOld(NetMessage* msg)
{
    const float ROUND_TRIP_TIME_MS = 150.0f;
    const uint32_t OLD_AGE_MS = (uint32_t)((ROUND_TRIP_TIME_MS + (ROUND_TRIP_TIME_MS * 0.1f)) / (ROUND_TRIP_TIME_MS * 0.2f));
    uint32_t age = (uint32_t)GetCurrentTimeMilliseconds() - msg->m_lastSentTimestampMs;
    return age > OLD_AGE_MS;
}

//-----------------------------------------------------------------------------------
bool NetConnection::CanAttachNewReliable()
{
    return true;
}

//-----------------------------------------------------------------------------------
uint16_t NetConnection::GetNextReliableID()
{
    uint16_t next = m_nextSentReliableId++;
    m_lastSentReliableId = next;
    return next;
}

//-----------------------------------------------------------------------------------
void NetConnection::MarkReliableConfirmed(const uint16_t reliableId)
{
    if (reliableId < m_oldestUnconfirmedReliableId)
    {
        return; //We already know it was confirmed--because it's below the oldest unconfirmed we have.
    }
    else if (reliableId == m_oldestUnconfirmedReliableId)
    {
        //The main reason we insert reliableId just to remove it, is to increment m_oldest correctly.
        auto ridIter = m_confirmedReliableIds.insert(reliableId).first; //Iter to inserted key.

        //This may not be as simple as just incrementing once.
        //e.g. m_oldestUnconfirmedReliableID was 3, and now we've received 2 for the first time.
        //So, we need to advance as many times as it takes to find m_oldest.
        while (ridIter != m_confirmedReliableIds.end())
        {
            m_confirmedReliableIds.erase(ridIter);
            ++m_oldestUnconfirmedReliableId; //Be sure it comes after the erase!
            ridIter = m_confirmedReliableIds.find(m_oldestUnconfirmedReliableId);
        } //Each time we remove at the oldest, we'll increment oldest for the next go-around, so we'll remove 0, 1, 2, but not 4 if find(3) fails.
    }
    else //The oldest is staying the oldest, but the new received id exceeds it. This above is when we receive 3, but still are missing 2.
    {
        m_confirmedReliableIds.emplace(reliableId);
    }
}

//-----------------------------------------------------------------------------------
bool NetConnection::IsReliableConfirmed(const uint16_t reliableId)
{
    if (reliableId < m_oldestUnconfirmedReliableId)
    {
        return true; //We already know it was confirmed--because it's below the oldest unconfirmed we have.
    }
    else
    {
        for (uint16_t id : m_confirmedReliableIds)
        {
            if (id == reliableId)
            {
                return true;
            }
        }
        return false;
    }
}

//-----------------------------------------------------------------------------------
bool NetConnection::CycleGreaterThanEqual(uint16_t a, uint16_t b)
{
    int16_t diff = a - b;
    return (diff > 0 && diff < 0x7fff);
}

//-----------------------------------------------------------------------------------
NetConnection::AckBundle* NetConnection::FindBundle(uint16_t ack)
{
    uint16_t idx = ack % MAX_ACK_BUNDLES;
    return &(m_ackBundles[idx]);
}

//-----------------------------------------------------------------------------------
bool NetConnection::HasReceivedReliable(const uint16_t reliableId)
{
    uint16_t min = m_nextExpectedReliableId - MAX_RELIABLE_RANGE;
    uint16_t max = m_nextExpectedReliableId;
    ASSERT_OR_DIE(reliableId <= (max + MAX_RELIABLE_RANGE), "Received a reliable outside of our expected max active reliables range");
    
    if (!CycleGreaterThanEqual(reliableId, min))
    {
        return true;
    }
    else if (CycleGreaterThanEqual(reliableId, m_nextExpectedReliableId))
    {
        return false;
    }
    else
    {
        for (uint16_t id : m_receivedReliableIds)
        {
            if (id == reliableId)
            {
                return true;
            }
        }
        return false;
    }
}

//-----------------------------------------------------------------------------------
bool NetConnection::CanProcessMessage(const NetMessage& msg)
{
    if (msg.IsReliable())
    {
        return !HasReceivedReliable(msg.m_reliableId);
    }
    else
    {
        return true;
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::MarkReliableReceived(const uint16_t reliableId)
{
    if (CycleGreaterThanEqual(reliableId, m_nextExpectedReliableId))
    {
        uint16_t distance = reliableId - m_nextExpectedReliableId;
        if (distance > MAX_RELIABLE_RANGE)
        {
            ERROR_AND_DIE("Received a reliable id outside of the window");
        }
        m_receivedReliableIds.push_back(reliableId);
        m_nextExpectedReliableId = reliableId + 1;
    }
    else
    {
        uint16_t diff = m_nextExpectedReliableId - reliableId;
        if (diff < MAX_RELIABLE_RANGE)
        {
            m_receivedReliableIds.push_back(reliableId);
        }
        if (m_nextExpectedReliableId == reliableId)
        {
            m_nextExpectedReliableId++;
        }
    }
    RemoveAllReliableIdsLessThan(reliableId - MAX_RELIABLE_RANGE);
}

//-----------------------------------------------------------------------------------
void NetConnection::RemoveAllReliableIdsLessThan(const uint16_t value)
{
    for (auto iter = m_receivedReliableIds.begin(); iter != m_receivedReliableIds.end();)
    {
        if (!CycleGreaterThanEqual(*iter, value))
        {
            iter = m_receivedReliableIds.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

//-----------------------------------------------------------------------------------
void NetConnection::AckBundle::AddReliable(uint16_t reliableId)
{
    sentReliableIds.push_back(reliableId);
}

//-----------------------------------------------------------------------------------
NetConnection::AckBundle* NetConnection::CreateBundle(uint16_t ack)
{
    uint16_t idx = ack % MAX_ACK_BUNDLES;
    AckBundle* bundle = &(m_ackBundles[idx]);
    bundle->ack = ack;
    bundle->reliableCount = 0;
    return bundle;
}