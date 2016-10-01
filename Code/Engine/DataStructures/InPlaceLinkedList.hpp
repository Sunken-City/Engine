#pragma once
#include "Engine\Core\ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------
template <typename T>
void AddInPlace(T*& list, T* nodeToAdd)
{
    if (list == nullptr) 
    {
        list = nodeToAdd;
        list->next = list->prev = list;
    }
    else 
    {
        nodeToAdd->prev = list->prev;
        nodeToAdd->next = list;
        list->prev->next = nodeToAdd;
        list->prev = nodeToAdd;
    }
}

//-----------------------------------------------------------------------------------
template <typename T>
void RemoveInPlace(T*& list, T* nodeToRemove)
{
    nodeToRemove->prev->next = nodeToRemove->next;
    nodeToRemove->next->prev = nodeToRemove->prev;
    if (nodeToRemove == list)
    {
        list = nodeToRemove->next == nodeToRemove ? nullptr : nodeToRemove->next;
    }
}

//-----------------------------------------------------------------------------------
template <typename T>
unsigned int GetCountInPlace(T* list)
{
    unsigned int numNodes = 1;
    T* current = list->next;
    while (current != list)
    {
        current = current->next;
        ++numNodes;
    }
    return numNodes;
}

//-----------------------------------------------------------------------------------
template <typename T>
void SortInPlace(T*& list, bool(ComparisonFunction)(T* first, T* second))
{
    bool isFinished = false;
    unsigned int numNodes = GetCountInPlace(list);

    T* curr = list;
    for (unsigned int i = 0; i < numNodes; ++i)
    {
        for (unsigned int j = i; j < numNodes - 1; ++j)
        {
            if (ComparisonFunction(curr, curr->next))
            {
                SwapInPlace(curr, curr->next);
            }
        }
        curr = curr->next;
    }
}

//-----------------------------------------------------------------------------------
template <typename T>
void SwapInPlace(T* first, T* second)
{
    ASSERT_OR_DIE(first != nullptr && second != nullptr, "Passed in a head or null node to swap");

    //Complete the swap
    if (AreAdjacentNodes(first, second))
    {
        T* tempPtr = first->prev;
        T* tempPtr2 = second->prev;
        first->prev = first->next;
        second->prev = tempPtr;
        first->next = second->next;
        second->next = tempPtr2;
    }
    else
    {
        T* tempPtr = first->next;
        first->next = second->next;
        second->next = tempPtr;
        tempPtr = first->prev;
        first->prev = second->prev;
        second->prev = tempPtr;
    }

    //Amend the surrounding nodes
    first->prev->next = first;
    first->next->prev = first;
    second->prev->next = second;
    second->next->prev = second;

}

//-----------------------------------------------------------------------------------
template <typename T>
bool AreAdjacentNodes(T* first, T* second)
{
    return (first->next == second && second->prev == first) || (first->prev == second && second->next == first);
}