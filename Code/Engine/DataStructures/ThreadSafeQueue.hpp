#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <deque>
#include "Engine/Core/Memory/UntrackedAllocator.hpp"

template <typename T>
class ThreadSafeQueue
{
public:

    //-----------------------------------------------------------------------------------
    ThreadSafeQueue()
    {
        InitializeCriticalSection(&m_criticalSection);
    }
    
    //-----------------------------------------------------------------------------------
    ~ThreadSafeQueue()
    {
        DeleteCriticalSection(&m_criticalSection);
    }

    //-----------------------------------------------------------------------------------
    void Enqueue(T* object)
    {
        EnterCriticalSection(&m_criticalSection);
        {
            m_queue.push_back(object);
        }
        LeaveCriticalSection(&m_criticalSection);
    }

    //-----------------------------------------------------------------------------------
    T* Dequeue()
    {
        T* front = nullptr;
        EnterCriticalSection(&m_criticalSection);
        {
            if (!m_queue.empty())
            {
                front = m_queue.front();
                m_queue.pop_front();
            }
        }
        LeaveCriticalSection(&m_criticalSection);
        return front;
    }

    //-----------------------------------------------------------------------------------
    unsigned int Size()
    {
        unsigned int size;
        EnterCriticalSection(&m_criticalSection);
        {
            size = m_queue.size();
        }
        LeaveCriticalSection(&m_criticalSection);
        return size;
    }

    //-----------------------------------------------------------------------------------
    T* Peek()
    {
        T* front = nullptr;
        EnterCriticalSection(&m_criticalSection);
        {
            front = m_queue.front();
        }
        LeaveCriticalSection(&m_criticalSection);
        return front;
    }

private:
    std::deque<T*, UntrackedAllocator<T*>> m_queue;
    CRITICAL_SECTION m_criticalSection;
};