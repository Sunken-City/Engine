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
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Execute(NamedProperties& params)
    {
        (*function)(params);
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    EventCallbackFunction* function;
};

//-----------------------------------------------------------------------------------
template<typename T_ObjectType, typename T_MethodType>
struct RegisteredObjectMethod : public RegisteredObjectBase
{
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Execute(NamedProperties& params)
    {
        (m_object->*m_method)(params);
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    T_ObjectType* m_object;
    T_MethodType* m_method;
};

//-----------------------------------------------------------------------------------
class EventSystem
{

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void RegisterEventCallback(const std::string& name, EventCallbackFunction function);
    void FireEvent(const std::string& name);

    template<typename T>
    void RegisterObjectForEvent(const std::string& eventName, T_ObjectType* object, T_MethodType* method)
    {
        RegisteredObjectMethod* rom = new RegisteredObjectMethod<T_ObjectType, T_MethodType>(object, method);
        //Find the eventName's vector
        //vector.push_back(rom);
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<std::string, std::vector<RegisteredObjectBase*>> m_registeredFunctions;
};