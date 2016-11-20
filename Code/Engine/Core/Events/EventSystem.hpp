#pragma once
#include "Engine/Core/Events/NamedProperties.hpp"
#include <map>
#include <string>
#include <vector>

typedef void (EventCallbackFunction)(NamedProperties& params);

//-----------------------------------------------------------------------------------
struct RegisteredObjectBase
{
    virtual void Execute(NamedProperties& params) = 0;
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

    //-----------------------------------------------------------------------------------
    template<typename T_ObjectType, typename T_MethodType>
    static void RegisterObjectForEvent(const std::string& eventName, T_ObjectType object, T_MethodType method, const char* usage = nullptr)
    {
        RegisteredObjectMethod<T_ObjectType, T_MethodType>* rom = new RegisteredObjectMethod<T_ObjectType, T_MethodType>(object, method, usage);
        s_registeredFunctions[eventName].push_back(rom);
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static std::map<std::string, std::vector<RegisteredObjectBase*>> s_registeredFunctions;
};

void FireEvent(const std::string& name, NamedProperties& namedProperties = NamedProperties::NONE);
void FireEvent(const char* name, NamedProperties& namedProperties = NamedProperties::NONE);