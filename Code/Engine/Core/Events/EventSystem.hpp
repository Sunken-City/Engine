#pragma once
#include "Engine/Core/Events/NamedProperties.hpp"
#include <map>
#include <string>
#include <vector>

typedef void (EventCallbackFunction)(NamedProperties& params);

//-----------------------------------------------------------------------------------
class EventSystem
{

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void RegisterEventCallback(const std::string& name, EventCallbackFunction function);
    void FireEvent(const std::string& name);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<std::string, std::vector<EventCallbackFunction>> m_registeredFunctions;
};