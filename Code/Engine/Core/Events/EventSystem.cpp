#include "Engine/Core/Events/EventSystem.hpp"

std::map<std::string, std::vector<RegisteredObjectBase*>, std::less<std::string>, UntrackedAllocator<std::pair<std::string, std::vector<RegisteredObjectBase*>>>>EventSystem::s_registeredFunctions;

//-----------------------------------------------------------------------------------
void EventSystem::FireEvent(const char* name, NamedProperties& namedProperties)
{
    std::vector<RegisteredObjectBase*> functions = EventSystem::s_registeredFunctions[std::string(name)];
    for (RegisteredObjectBase* callee : functions)
    {
        callee->Execute(namedProperties);
    }
}

//-----------------------------------------------------------------------------------
void EventSystem::FireEvent(const std::string& name, NamedProperties& namedProperties)
{
    std::vector<RegisteredObjectBase*> functions = EventSystem::s_registeredFunctions[name];
    for (RegisteredObjectBase* callee : functions)
    {
        callee->Execute(namedProperties);
    }
}

//-----------------------------------------------------------------------------------
void EventSystem::CleanUpEventRegistry()
{
    for (auto eventPair : s_registeredFunctions)
    {
        std::vector<RegisteredObjectBase*>& subscribers = eventPair.second;
        for (auto iter = subscribers.begin(); iter != subscribers.end();)
        {
            RegisteredObjectBase* rob = *iter;
            iter = subscribers.erase(iter);
            delete rob;
        }
        subscribers.clear();
    }
    s_registeredFunctions.clear();
}

//-----------------------------------------------------------------------------------
void EventSystem::RegisterEventCallback(const std::string& eventName, EventCallbackFunction* m_function, const char* usage)
{
    RegisteredFunction* rom = new RegisteredFunction(m_function, usage);
    s_registeredFunctions[eventName].push_back(rom);
}