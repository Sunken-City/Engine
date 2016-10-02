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
//Returns the first element in the list, which is the list.
template <typename T>
T* GetFirst(T* list)
{
    return list;
}

//-----------------------------------------------------------------------------------
//Returns the last element in the list, relative to the first.
template <typename T>
T* GetLast(T* list)
{
    return (list) == nullptr ? nullptr : list->prev;
}

//-----------------------------------------------------------------------------------
//Return the previous element without looping(so if elem is the first element, then return nullptr).
template <typename T>
T* GetPreviousWithoutLooping(T* list, T* element)
{
     return ((element == nullptr) || (element == GetFirst(list))) ? nullptr : element->prev;
}

//------------------------------------------------------------------------
template <typename T, typename COMPARE>
T* SortInPlace(T*& list, COMPARE comparisonFunction)
{
    T* i = GetFirst(list);

    // Keeps track at which pointer in the list is the end of the sorted
    // part of this list.  Let's us know when to stop swapping
    T* sortedLast = nullptr;

    // Keep track if I should early out
    // as soon as I can move through it without swapping, I know it is sorted.
    bool swapped = true;

    // If I'm at the last, then I'm basically done (only one element to sort, and no way
    // that is unsorted.
    while (swapped && (sortedLast != GetLast(list)))
    {
        swapped = false;

        // Get the second to last element
        T* j = GetLast(list);
        j = GetPreviousWithoutLooping(list, j);

        while (j != sortedLast) 
        {
            // pre-cach the next previous iteration
            // since j might move around.
            T* p = GetPreviousWithoutLooping(list, j);
            T* n = j->next;

            // If this is greater, means j is greater than next (so out of order) 
            if (comparisonFunction(j, n) > 0) 
            {
                //SWAP
                swapped = true;

                // Different than p, since LLP_LIST_PREV will return NULLPTR if it is at the beginning)
                T* prev = j->prev;
                T* next = n->next;

                // Fix up links
                prev->next = n;
                n->prev = prev;
                n->next = j;
                j->prev = n;
                j->next = next;
                next->prev = j;

                // Update list pointer
                if (list == j) 
                {
                    list = n;
                }
            }

            // update my iterator
            j = p;
        }

        // update our previous pointer
        if (sortedLast == nullptr) 
        {
            sortedLast = GetFirst(list);
        }
        else 
        {
            sortedLast = sortedLast->next;
        }
    }

    return list;
}


//-----------------------------------------------------------------------------------
template <typename T>
bool AreAdjacentNodes(T* first, T* second)
{
    return (first->next == second && second->prev == first) || (first->prev == second && second->next == first);
}