#pragma once

template <typename T>
void AddInPlace(T* &list, T* nodeToAdd)
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

template <typename T>
void RemoveInPlace(T* &list, T* nodeToRemove)
{
    nodeToRemove->prev->next = nodeToRemove->next;
    nodeToRemove->next->prev = nodeToRemove->prev;
    if (nodeToRemove == list)
    {
        list = nodeToRemove->next == nodeToRemove ? nullptr : nodeToRemove->next;
    }
}