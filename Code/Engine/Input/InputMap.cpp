#include "Engine/Input/InputMap.hpp"
#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------------
InputMap::InputMap()
{

}

//-----------------------------------------------------------------------------------
InputMap::~InputMap()
{
    Clear();
}

//-----------------------------------------------------------------------------------
InputValue* InputMap::MapInputValue(std::string const &name)
{
    InputValue *val = FindInputValue(name);

    //If we don't find it, create it.
    if (val == nullptr) 
    {
        val = new InputValue(this);
        m_values[name] = val;
    }
    return val;
}

//-----------------------------------------------------------------------------------
InputValue* InputMap::MapInputValue(const std::string& name, InputValue* other)
{
    InputValue* val = MapInputValue(name);
    val->AddMapping(other);
    return val;
}

//-----------------------------------------------------------------------------------
InputValue* InputMap::FindInputValue(std::string const &name)
{
    auto it = m_values.find(name);
    return (m_values.find(name) == m_values.end()) ? nullptr : it->second;
}

//-----------------------------------------------------------------------------------
float InputMap::GetValue(const std::string& name)
{
    InputValue* val = FindInputValue(name);
    return (nullptr != val) ? val->GetValue() : 0.0f;
}

//-----------------------------------------------------------------------------------
InputAxis* InputMap::MapInputAxis(std::string const &name)
{
    InputAxis* axis = FindInputAxis(name);

    //If we didn't find it, create it.
    if (axis == nullptr)
    {
        axis = new InputAxis(this);
        m_axies[name] = axis;
    }
    return axis;
}

//-----------------------------------------------------------------------------------
InputAxis* InputMap::MapInputAxis(const std::string& name, VirtualInputValue& positiveInput, VirtualInputValue& negativeInput)
{
    InputAxis* axis = MapInputAxis(name);
    axis->AddMapping(positiveInput, negativeInput);
    return axis;
}

//-----------------------------------------------------------------------------------
InputAxis* InputMap::MapInputAxis(const std::string& name, InputAxis* inputAxis)
{
    InputAxis* axis = MapInputAxis(name);
    axis->AddMapping(inputAxis->m_positiveValue, inputAxis->m_negativeValue);
    return axis;
}

//-----------------------------------------------------------------------------------
InputAxis* InputMap::FindInputAxis(std::string const &name)
{
    auto it = m_axies.find(name);
    return (m_axies.find(name) == m_axies.end()) ? nullptr : it->second;
}

//-----------------------------------------------------------------------------------
Vector2 InputMap::GetVector2(const std::string& xName, const std::string& yName)
{
    InputAxis* x = FindInputAxis(xName);
    InputAxis* y = FindInputAxis(yName);
    return Vector2(x->GetValue(), y->GetValue());
}

//-----------------------------------------------------------------------------------
void InputMap::Clear()
{
    for (auto valuePair : m_values)
    {
        delete valuePair.second;
    }
    for (auto axisPair : m_axies)
    {
        delete axisPair.second;
    }
}

//-----------------------------------------------------------------------------------
bool InputMap::WasJustReleased(const std::string& name)
{
    InputValue* val = FindInputValue(name);
    return val ? val->WasJustReleased() : false;
}

//-----------------------------------------------------------------------------------
bool InputMap::WasJustPressed(const std::string& name)
{
    InputValue* val = FindInputValue(name);
    return val ? val->WasJustPressed() : false;
}

//-----------------------------------------------------------------------------------
bool InputMap::IsDown(const std::string& name)
{
    InputValue* val = FindInputValue(name);
    return val ? val->IsDown() : false;
}

//-----------------------------------------------------------------------------------
bool InputMap::IsUp(const std::string& name)
{
    InputValue* val = FindInputValue(name);
    return val ? val->IsUp() : true;
}
