#pragma once

typedef unsigned char byte;

//If src is [1,2,3,4,5,6,7], dest should be [7,6,5,4,3,2,1]
void MemcpyBackwards(void* dest, const void* src, const size_t count)
{
    byte* pos = (byte*)dest;
    byte* read = src + count;
    for (size_t index = 0; index < count; ++index)
    {
        --read;
        *((byte*)pos) = *((byte*)read);
        ++pos;
    }
}