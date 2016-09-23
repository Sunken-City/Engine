#pragma once
#include <stdio.h>
#include <stdint.h>

typedef unsigned char byte;

class IBinaryReader
{
public:
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum Endianness : byte
    {
        LITTLE_ENDIAN,
        BIG_ENDIAN,
        NUM_MODES
    };

    IBinaryReader() : m_endianMode(LITTLE_ENDIAN) {};
    IBinaryReader(Endianness endianness) : m_endianMode(endianness) {};

    //GETTERS//////////////////////////////////////////////////////////////////////////
    Endianness GetLocalEndianess();

    //SETTERS//////////////////////////////////////////////////////////////////////////
    inline void SetEndianess(Endianness mode) { m_endianMode = mode; };

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    size_t ReadString(const char*& stringBuffer, size_t bufferSize);
    //Returns the number of bytes written. This is the core implementation that subclasses
    //need to support. Writes to the appropriate buffer the bytes.
    virtual void* ReadBytes(const size_t numBytes) = 0;

    //-----------------------------------------------------------------------------------
    template<typename T>
    void ByteSwap(T* source, const size_t numBytes)
    {
        byte* data = (byte*)source;
        byte* start = data;
        byte* end = data + numBytes - 1;

        while (start < end)
        {
            byte temp = *start;
            *start = *end;
            *end = temp;
            ++start;
            --end;
        }

        source = (T*)data;
    }

    //-----------------------------------------------------------------------------------
    template<typename T>
    bool Read(T& data)
    {
        void* readData = ReadBytes(sizeof(T));
        memcpy(&data, readData, sizeof(T));
        delete readData;
        if (GetLocalEndianess() != m_endianMode)
        {
            ByteSwap(&data, sizeof(T));
        }
        return true;
    }
    
private:
    Endianness m_endianMode;
};

class BinaryFileReader : public IBinaryReader
{
public:
    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    bool Open(const char* filePath);
    void Close();
    virtual void* ReadBytes(const size_t numBytes) override;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    FILE* fileHandle;
};