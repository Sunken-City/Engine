#include "Engine/Net/UDPIP/PacketChannel.hpp"
#include "Engine/Time/Time.hpp"

//-----------------------------------------------------------------------------------
PacketChannel::PacketChannel()
    : m_additionalLagMilliseconds(0.0f, 0.0f)
    , m_dropRate(0.0f)
    , m_pool(2048)
{

}

//-----------------------------------------------------------------------------------
PacketChannel::~PacketChannel()
{

}

//-----------------------------------------------------------------------------------
size_t PacketChannel::SendTo(const sockaddr_in& toAddress, void const* data, const size_t dataSize)
{
    return m_socket.SendTo(toAddress, data, dataSize);
}

//-----------------------------------------------------------------------------------
void PacketChannel::ReceiveOffSocket(sockaddr_in& fromAddress)
{
    size_t read = 0;
    do 
    {
        TimeStampedPacket* timeStamped = m_pool.Alloc<TimeStampedPacket>();
        read = m_socket.RecieveFrom(fromAddress, timeStamped->packet.m_buffer);
        timeStamped->packet.m_fromAddress = fromAddress;
        if (read > 0)
        {
            if (MathUtils::GetRandomFloatFromZeroTo(1.0f) < m_dropRate)
            {
                m_pool.Free(timeStamped);
            }
            else
            {
                double delay = m_additionalLagMilliseconds.GetRandom();
                timeStamped->packet.SetReadableBytes(read);
                timeStamped->timeToProcess = GetCurrentTimeMilliseconds() + delay;
                m_inboundPackets.Enqueue(timeStamped);
            }
        }
        else
        {
            m_pool.Free(timeStamped);
        }
    } while (read > 0);
}

//-----------------------------------------------------------------------------------
size_t PacketChannel::RecieveFrom(sockaddr_in& fromAddress, void* buffer)
{
    ReceiveOffSocket(fromAddress);
    if (m_inboundPackets.Size() > 0)
    {
        double curentTimeMilliseconds = GetCurrentTimeMilliseconds();
        if (curentTimeMilliseconds >= m_inboundPackets.Peek()->timeToProcess)
        {
            TimeStampedPacket* tsp = m_inboundPackets.Dequeue();
            fromAddress = tsp->packet.m_fromAddress;
            size_t size = tsp->packet.GetTotalReadableBytes();
            memcpy(buffer, tsp->packet.m_buffer, size);
            m_pool.Free(tsp);
            return size;
        }
    }
    return 0;

}

//-----------------------------------------------------------------------------------
sockaddr_in PacketChannel::GetAddress()
{
    return m_socket.m_address;
}

