#include "Engine/Core/Events/EventSystem.hpp"


std::map<std::string, std::vector<RegisteredObjectBase*>> EventSystem::s_registeredFunctions;

//-----------------------------------------------------------------------------------
void FireEvent(const char* name, NamedProperties& namedProperties)
{
    std::vector<RegisteredObjectBase*> functions = EventSystem::s_registeredFunctions[std::string(name)];
    for (RegisteredObjectBase* callee : functions)
    {
        callee->Execute(namedProperties);
    }
}

//-----------------------------------------------------------------------------------
void FireEvent(const std::string& name)
{

}

//-----------------------------------------------------------------------------------
void EventSystem::RegisterEventCallback(const std::string& eventName, EventCallbackFunction* m_function)
{
    RegisteredFunction* rom = new RegisteredFunction(m_function);
    s_registeredFunctions[eventName].push_back(rom);
}