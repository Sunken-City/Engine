#include "Engine/DataStructures/BytePacker.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------
BytePacker::BytePacker(void* buffer, size_t writeSizeMax, size_t readSizeMax, IBinaryReader::Endianness endianness)
    : IBinaryReader(endianness)
    , IBinaryWriter((IBinaryWriter::Endianness)endianness)
    , m_buffer(buffer)
    , m_writeSizeMax(writeSizeMax)
    , m_readSizeMax(readSizeMax)
    , m_offset(0)
{

}

//-----------------------------------------------------------------------------------
BytePacker::BytePacker()
    : IBinaryReader(IBinaryReader::Endianness::BIG_ENDIAN)
    , IBinaryWriter(IBinaryWriter::Endianness::BIG_ENDIAN)
    , m_buffer(nullptr)
    , m_writeSizeMax(0)
    , m_readSizeMax(0)
    , m_offset(0)
{
}

//-----------------------------------------------------------------------------------
void* BytePacker::ReadBytes(const size_t numBytes)
{
    if (numBytes < 0)
    {
        return nullptr;
    }
    void* buffer = new byte[numBytes];
    memcpy(buffer, (void*)((size_t)m_buffer + m_offset), numBytes);
    m_readSizeMax -= numBytes;
    m_offset += numBytes;
    return buffer;
}

//-----------------------------------------------------------------------------------
void BytePacker::ReadBytes(void* dest, const size_t numBytes)
{
    if (numBytes == 0)
    {
        return;
    }
    memcpy(dest, (void*)((size_t)m_buffer + m_offset), numBytes);
    m_readSizeMax -= numBytes;
    m_offset += numBytes;
}

//-----------------------------------------------------------------------------------
const char* BytePacker::ReadString()
{
    size_t numBytes = GetReadableBytes();
    if (numBytes < 1)
    {
        return nullptr;
    }
    byte firstByte = 0xFF;
    if (!Read<byte>(firstByte) || firstByte == 0xFF)
    {
        return nullptr;
    }
    else
    {
        char* buffer = (char*)GetHead() - 1;
        size_t max_size = GetReadableBytes() + 1;
        size_t length = 0;
        char* currentCharacter = buffer;
        while ((length < max_size) && (*currentCharacter != NULL))
        {
            ++length;
            ++currentCharacter;
        }
        if (length <= m_readSizeMax)
        {
            Advance(length);
            return buffer;
        }
        else
        {
            return nullptr;
        }
    }
}

//-----------------------------------------------------------------------------------
size_t BytePacker::WriteBytes(const void* data, const size_t dataSize)
{
    if (dataSize == 0)
    {
        return 0;
    }
    ASSERT_OR_DIE(m_writeSizeMax >= dataSize, "Attempted to write more than we had room for");
    memcpy((void*)((size_t)m_buffer + m_offset), data, dataSize);
    m_writeSizeMax -= dataSize;
    m_offset += dataSize;
    return dataSize;
}

//-----------------------------------------------------------------------------------
void BytePacker::WriteString(const char* str)
{
    if (str == nullptr)
    {
        Write<byte>(0xFF); //Always null in ASCII
    }
    else
    {
        size_t length = strlen(str) + 1;
        WriteBytes(str, length);
    }
}

//-----------------------------------------------------------------------------------
void BytePacker::Advance(size_t offset)
{
    m_offset += offset;
}

//-----------------------------------------------------------------------------------
void* BytePacker::GetHead()
{
    return (void*)((size_t)m_buffer + m_offset);
}

//-----------------------------------------------------------------------------------
void BytePacker::SetReadableBytes(size_t readSizeMax)
{
    m_readSizeMax = readSizeMax;
    m_offset = 0;
}
