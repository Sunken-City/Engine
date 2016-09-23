#pragma once
#include <algorithm>
#include "Engine/Input/BinaryReader.hpp"
#include "Engine/Input/BinaryWriter.hpp"

typedef unsigned char byte;

//-----------------------------------------------------------------------------------
class BytePacker : public IBinaryReader, public IBinaryWriter
{
public:
    BytePacker();
    BytePacker(void* buffer, size_t writeSizeMax, size_t readSizeMax, IBinaryReader::Endianness endianness);


    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void* ReadBytes(const size_t numBytes);
    const char* ReadString();
    void ReadBytes(void* dest, const size_t numBytes);
    size_t WriteBytes(const void* src, const size_t numBytes) override;
    void WriteString(const char* str);
    void Advance(size_t offset);
    void* GetHead();
    void SetReadableBytes(size_t readSizeMax);
    size_t GetReadableBytes() const { return GetTotalReadableBytes(); };
    inline size_t GetTotalReadableBytes() const { return std::max<size_t>(m_offset, m_readSizeMax); };
    size_t GetWritableBytes() const { return GetTotalWritableBytes() - m_offset; };
    inline size_t GetTotalWritableBytes() const { return std::max<size_t>(m_offset, m_writeSizeMax); };

    //-----------------------------------------------------------------------------------
    template<typename T>
    T* Reserve(const T& data)
    {
        T* bookmark = (T*)((size_t)m_buffer + m_offset);
        Write<T>(data);
        return bookmark;
    }

    void* m_buffer;
    size_t m_writeSizeMax; //buffer size
    size_t m_readSizeMax;

private:
    size_t m_offset; //write & read offset
};