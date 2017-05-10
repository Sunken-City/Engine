#include "Engine/Renderer/UniformBuffer.hpp"
#include "Renderer.hpp"
#include "OpenGLExtensions.hpp"

int UniformBuffer::COPY_BUFFER_INDEX = -1;

//-----------------------------------------------------------------------------------
UniformBuffer::UniformBuffer(void* data, size_t dataSize)
    : m_data(data)
    , m_dataSize(dataSize)
{
    m_bufferHandle = Renderer::instance->CreateRenderBuffer(dataSize, data);

    if (COPY_BUFFER_INDEX == 0)
    {
        GLint maxBindPoint;
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxBindPoint);
        COPY_BUFFER_INDEX = maxBindPoint - 1;
    }
}

//-----------------------------------------------------------------------------------
UniformBuffer::~UniformBuffer()
{

}

//-----------------------------------------------------------------------------------
void UniformBuffer::UpdateData(void* data)
{
    ASSERT_RECOVERABLE(data != m_data, "Updated a uniform buffer with the same data");
    m_data = memcpy(m_data, data, m_dataSize);
    m_isDirty = true;
}

//-----------------------------------------------------------------------------------
void UniformBuffer::CopyToGPU()
{
    if (m_isDirty)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, COPY_BUFFER_INDEX, m_bufferHandle); //Replace this with glNamedBufferData, because we don't care where we bind this to manipulate.
        glBufferData(GL_UNIFORM_BUFFER, m_dataSize, m_data, GL_DYNAMIC_DRAW);
        m_isDirty = false;
    }
}
