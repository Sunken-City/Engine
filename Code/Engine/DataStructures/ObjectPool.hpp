#pragma once

template <typename T>
class ObjectPool
{
    struct PoolNode
    {
        PoolNode* next;
    };

public:
    //-----------------------------------------------------------------------------------
    ObjectPool(size_t poolSize)
    {
        m_objects = (T*)malloc(poolSize * sizeof(T));

        m_freeList = nullptr;
        for (size_t i = 0; i < poolSize; ++i) {
            T *obj = m_objects + i;

            PoolNode *node = (PoolNode*)obj;
            node->next = m_freeList;
            m_freeList = node;
        }
    }

    //-----------------------------------------------------------------------------------
    ~ObjectPool()
    {
        free(m_objects);
    }

    //-----------------------------------------------------------------------------------
    template <typename T, typename ...ARGS>
    T* Alloc(ARGS... args)
    {
        T *obj = (T*)m_freeList;
        m_freeList = m_freeList->next;

        new (obj) T(args...);
        return obj;
    }

    //-----------------------------------------------------------------------------------
    void Free(T* obj)
    {
        obj->~T();

        PoolNode* node = (PoolNode*)obj;
        node->next = m_freeList;
        m_freeList = node;
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    T* m_objects;
    PoolNode* m_freeList;
};