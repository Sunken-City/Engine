#pragma once

#include <vector>
#include "Engine\Core\Memory\UntrackedAllocator.hpp"

template <typename ...Args>
class Event
{
public:
    struct Subscription;
    //TYPEDEFS/////////////////////////////////////////////////////////////////////
    typedef void(SubscriptionCallback)(Subscription* sub, Args...);
    typedef void(FunctionCallback)(Args...);
    typedef void(EventCallback)(Subscription* sub, Args...);

    //STRUCTS/////////////////////////////////////////////////////////////////////
    template <typename CALLBACKTYPE>
    struct SubscriptionType
    {
        SubscriptionCallback* utilityCallback; // This is the C style function the trigger actually calls
        void* argument;         // Additional data that goes with the subscription (optional)

        union {
            CALLBACKTYPE callback;         // Is the callback registered by the user
            char callback_data[16];
        };
    };

    struct Subscription : public SubscriptionType<void*> {};

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Event() {};

    //-----------------------------------------------------------------------------------
    ~Event()
    {
        UnregisterAllSubscriptions();
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void RegisterFunction(FunctionCallback* cb)
    {
        RegisterSubscription(&EventFunctionCallback, cb, nullptr);
    }

    //-----------------------------------------------------------------------------------
    void UnregisterFunction(FunctionCallback* cb)
    {
        UnregisterSubscription(&EventFunctionCallback, cb, nullptr);
    }

    //-----------------------------------------------------------------------------------
    template <typename T>
    void RegisterMethod(T* object, void (T::*methodCallback)(Args...))
    {
        RegisterSubscription(&MethodCallback<T, decltype(methodCallback)>, methodCallback, object);
    }

    //-----------------------------------------------------------------------------------
    template <typename T>
    void UnregisterMethod(T* object, void (T::*methodCallback)(Args...))
    {
        UnregisterSubscription(&MethodCallback<T, decltype(methodCallback)>, methodCallback, object);
    }

    //-----------------------------------------------------------------------------------
    void UnregisterAllSubscriptions()
    {
        m_subscriptions.clear();
    }

    //-----------------------------------------------------------------------------------
    void Trigger(Args... args)
    {
        unsigned int size = m_subscriptions.size();
        for (unsigned int i = 0; i < size; ++i)
        {
            auto& sub = m_subscriptions[i];
            sub.utilityCallback(&sub, args...);
            if (m_subscriptions.size() < size)
            {
                size = m_subscriptions.size();
                --i;
            }
        }
    }

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    template <typename T, typename METHODTYPE>
    static void MethodCallback(SubscriptionType<METHODTYPE>* sub, Args... args)
    {
        METHODTYPE mcb = sub->callback;
        T* objectPtr = (T*)sub->argument;
        (objectPtr->*mcb)(args...);
    }

    //-----------------------------------------------------------------------------------
    static void EventFunctionCallback(Subscription* sub, Args... args)
    {
        FunctionCallback* cb = (FunctionCallback*)(sub->callback);
        cb(args...);
    }

private:
    //PRIVATE FUNCTIONS/////////////////////////////////////////////////////////////////////
    template <typename CBTYPE>
    void RegisterSubscription(void* utilityCallback, CBTYPE actualCallback, void* data)
    {
        SubscriptionType<CBTYPE> sub;
        sub.utilityCallback = (SubscriptionCallback*)utilityCallback;
        sub.callback = actualCallback;
        sub.argument = data;

        Subscription subConv;
        subConv = *(Subscription*)&sub;

        m_subscriptions.push_back(subConv);
    }

    //-----------------------------------------------------------------------------------
    template <typename CBTYPE>
    void UnregisterSubscription(void* utilityCallback, CBTYPE actualCallback, void* data)
    {
        for (auto subIter = m_subscriptions.begin(); subIter != m_subscriptions.end(); ++subIter)
        {
            SubscriptionType<CBTYPE> *sub = (SubscriptionType<CBTYPE>*) &(*subIter);
            if (sub->callback == actualCallback && sub->utilityCallback == utilityCallback && sub->argument == data)
            {
                m_subscriptions.erase(subIter);
                break;
            }
        }
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Subscription, UntrackedAllocator<Subscription>> m_subscriptions;
};