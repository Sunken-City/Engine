#pragma once

template <typename T>
class RingBuffer
{
public:
    RingBuffer(unsigned int bufferSize);
    ~RingBuffer();
    void Push(const T& object);
    T Pop();
    T Peek();

private:
    T* m_bufferBegin;
    T* m_bufferEnd;
    T* m_head;
    T* m_tail;
    unsigned int m_maxSize;
    unsigned int m_currentSize;
};

//-----------------------------------------------------------------------------------
template <typename T>
RingBuffer<T>::RingBuffer(unsigned int bufferSize)
    : m_bufferBegin(nullptr)
    , m_bufferEnd(nullptr)
    , m_head(nullptr)
    , m_tail(nullptr)
    , m_maxSize(bufferSize)
    , m_currentSize(0)
{
    m_bufferBegin = static_cast<T*>(malloc(bufferSize * sizeof(T)));
    m_bufferEnd = m_bufferBegin + bufferSize;
    m_head = m_bufferBegin;
    m_tail = m_bufferBegin;
}

//-----------------------------------------------------------------------------------
template <typename T>
RingBuffer<T>::~RingBuffer()
{
    free(m_bufferBegin);
}

//-----------------------------------------------------------------------------------
template <typename T>
void RingBuffer<T>::Push(const T& object)
{
    //TODO: return a bool
    *m_head = object;
    ++m_head;
    ++m_currentSize;
    if (m_head == m_bufferEnd)
    {
        m_head = m_bufferBegin;
    }
}

//-----------------------------------------------------------------------------------
template <typename T>
T RingBuffer<T>::Pop()
{
    T* data = m_tail;
    ++m_tail;
    --m_currentSize;
    if (m_tail == m_bufferEnd)
    {
        m_tail = m_bufferBegin;
    }
    return *data;
}

//-----------------------------------------------------------------------------------
template <typename T>
T RingBuffer<T>::Peek()
{
    return *m_tail;
}
