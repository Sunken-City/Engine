#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <queue>
#include <vector>
#include "Engine/Core/Memory/UntrackedAllocator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

template <typename T, typename Comparator>
class ThreadSafePriorityQueue
{
public:

    //-----------------------------------------------------------------------------------
    ThreadSafePriorityQueue()
    {
        InitializeCriticalSection(&m_criticalSection);
    }

    //-----------------------------------------------------------------------------------
    ~ThreadSafePriorityQueue()
    {
        DeleteCriticalSection(&m_criticalSection);
    }

    //-----------------------------------------------------------------------------------
    void Enqueue(const T& object)
    {
        EnterCriticalSection(&m_criticalSection);
        {
            m_queue.push(object);
        }
        LeaveCriticalSection(&m_criticalSection);
    }

    //-----------------------------------------------------------------------------------
    T Dequeue()
    {
        EnterCriticalSection(&m_criticalSection);
            ASSERT_OR_DIE(!m_queue.empty(), "Attempted to pop from the priority queue, but it was empty.");
            T front = m_queue.top();
            m_queue.pop();
        LeaveCriticalSection(&m_criticalSection);
        return front;
    }

    //-----------------------------------------------------------------------------------
    bool IsEmpty()
    {
        bool isEmpty = true;
        EnterCriticalSection(&m_criticalSection);
        {
            isEmpty = m_queue.empty();
        }
        LeaveCriticalSection(&m_criticalSection);
        return isEmpty
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
    T Peek()
    {
        EnterCriticalSection(&m_criticalSection);
            T front = m_queue.top();
        LeaveCriticalSection(&m_criticalSection);
        return front;
    }

private:
    std::priority_queue<T, std::vector<T, UntrackedAllocator<T>>, Comparator> m_queue;
    CRITICAL_SECTION m_criticalSection;
};