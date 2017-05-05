#pragma once

class UniformBuffer
{
public:
    enum Index : unsigned int
    {
        FRAME_BLOCK = 0,
        MODEL_BLOCK = 1,
        MATERIAL_BLOCK = 2,
        CAMERA_BLOCK = 3,
        LIGHTING_BLOCK = 4,

        USER_BLOCK = 10
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    UniformBuffer(void* data, size_t dataSize);
    ~UniformBuffer();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void UpdateData(void* data);
    void CopyToGPU();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static int COPY_BUFFER_INDEX;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    void* m_data;
    size_t m_dataSize;
    unsigned int m_bufferHandle = 0; //GPU representation
    bool m_isDirty = true;
};