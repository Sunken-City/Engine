#pragma once
#include "Engine/Net/UDPIP/NetPacket.hpp"
#include "Engine/DataStructures/ObjectPool.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Net/UDPIP/UDPSocket.hpp"
#include "Engine/DataStructures/ThreadSafePriorityQueue.hpp"

//-----------------------------------------------------------------------------------
struct TimeStampedPacket
{
    NetPacket packet;
    double timeToProcess;
};

//-----------------------------------------------------------------------------------
class TimeStampedPacketComparison
{
public:
    TimeStampedPacketComparison(const bool& reversedSort = false)
    {
        reverse = reversedSort;
    }
    bool operator() (const TimeStampedPacket& lhs, const TimeStampedPacket& rhs) const
    {
        if (reverse) return (lhs.timeToProcess < rhs.timeToProcess);
        else return (lhs.timeToProcess > rhs.timeToProcess);
    }
    bool operator() (const TimeStampedPacket* lhs, const TimeStampedPacket* rhs) const
    {
        if (reverse) return (lhs->timeToProcess < rhs->timeToProcess);
        else return (lhs->timeToProcess > rhs->timeToProcess);
    }

    bool reverse;
};

//-----------------------------------------------------------------------------------
class PacketChannel
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    PacketChannel();
    ~PacketChannel();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    inline void Bind(const char* address, const char* portNumber) { m_socket.Bind(address, portNumber); };
    inline void Unbind() { m_socket.Unbind(); };
    inline bool IsBound() { return m_socket.IsBound(); };
    size_t SendTo(const sockaddr_in& toAddress, void const* data, const size_t dataSize);
    size_t RecieveFrom(sockaddr_in& fromAddress, void* buffer);
    sockaddr_in GetAddress();
    void ReceiveOffSocket(sockaddr_in& fromAddress);
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    UDPSocket m_socket;
    float m_dropRate;
    ThreadSafePriorityQueue<TimeStampedPacket*, TimeStampedPacketComparison> m_inboundPackets;
    Range<double> m_additionalLagMilliseconds;
    ObjectPool<TimeStampedPacket> m_pool;
};