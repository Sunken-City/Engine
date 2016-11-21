#pragma once
#include "Engine/Core/Events/NamedProperties.hpp"
#include <map>
#include <string>
#include <vector>
#include "../Memory/UntrackedAllocator.hpp"

typedef void (EventCallbackFunction)(NamedProperties& params);

//-----------------------------------------------------------------------------------
struct RegisteredObjectBase
{
    virtual void Execute(NamedProperties& params) = 0;
    virtual void* GetOwningObject() { return nullptr; };
};

//-----------------------------------------------------------------------------------
struct RegisteredFunction : public RegisteredObjectBase
{
    RegisteredFunction(EventCallbackFunction* func, const char* usage = nullptr) : m_function(func), m_usage(usage) {};
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Execute(NamedProperties& params)
    {
        (*m_function)(params);
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    EventCallbackFunction* m_function;
    const char* m_usage;
};

//-----------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_MethodType>
struct RegisteredObjectMethod : public RegisteredObjectBase
{
    RegisteredObjectMethod(T_ObjectType obj, T_MethodType method, const char* usage = nullptr) : m_object(obj), m_method(method), m_usage(usage) {};
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Execute(NamedProperties& params)
    {
        (m_object->*m_method)(params);
    }

    virtual void* GetOwningObject() { return (void*)m_object; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    T_ObjectType m_object;
    T_MethodType m_method;
    const char* m_usage;
};

//-----------------------------------------------------------------------------------
class EventSystem
{
public:
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    static void RegisterEventCallback(const std::string& eventName, EventCallbackFunction* m_function, const char* usage = nullptr);
    static void FireEvent(const std::string& name, NamedProperties& namedProperties = NamedProperties::NONE);
    static void FireEvent(const char* name, NamedProperties& namedProperties = NamedProperties::NONE);
    static void CleanUpEventRegistry();

    //-----------------------------------------------------------------------------------
    template<typename T_ObjectType>
    static void UnregisterFromEvent(const std::string& eventName, T_ObjectType object)
    {
        std::vector<RegisteredObjectBase*>& subscribers = s_registeredFunctions[eventName];
        for (auto iter = subscribers.begin(); iter != subscribers.end();)
        {
            RegisteredObjectBase* rob = *iter;
            void* owningObject = rob->GetOwningObject();
            if (static_cast<void*>(object) == owningObject)
            {
                iter = subscribers.erase(iter);
                delete rob;
            }
            else
            {
                ++iter;
            }
        }
    }

    //-----------------------------------------------------------------------------------
    template<typename T_ObjectType>
    static void UnregisterFromAllEvents(T_ObjectType object)
    {
        for (auto eventPair : s_registeredFunctions)
        {
            UnregisterFromEvent<T_ObjectType>(eventPair.first, object);
        }
    }

    //-----------------------------------------------------------------------------------
    template<typename T_ObjectType, typename T_MethodType>
    static void RegisterObjectForEvent(const std::string& eventName, T_ObjectType object, T_MethodType method, const char* usage = nullptr)
    {
        RegisteredObjectMethod<T_ObjectType, T_MethodType>* rom = new RegisteredObjectMethod<T_ObjectType, T_MethodType>(object, method, usage);
        s_registeredFunctions[eventName].push_back(rom);
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static std::map<std::string, std::vector<RegisteredObjectBase*>, std::less<std::string>, UntrackedAllocator<std::pair<std::string, std::vector<RegisteredObjectBase*>>>> s_registeredFunctions;
};
