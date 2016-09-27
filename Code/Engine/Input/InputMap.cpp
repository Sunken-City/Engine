#include "Engine/Input/InputMap.hpp"
#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"

InputMap::InputMap()
{

}

InputMap::~InputMap()
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

InputValue* InputMap::AddInputValue(std::string const &name)
{
    InputValue *val = nullptr;

    // If don't find it, create it.
    auto it = m_values.find(name);
    if (m_values.find(name) == m_values.end()) 
    {
        val = new InputValue(this);
        m_values[name] = val;
    }
    else 
    {
        val = it->second;
    }

    return val;
}

InputValue* InputMap::AddInputValue(const std::string& name, InputValue* other)
{
    InputValue* val = AddInputValue(name);
    val->AddMapping(other);
    return val;
}

InputValue* InputMap::FindInputValue(std::string const &name)
{
    // If don't find it, create it.
    auto it = m_values.find(name);
    if (m_values.find(name) == m_values.end()) 
    {
        return nullptr;
    }

    return it->second;
}

float InputMap::GetValue(const std::string& name)
{
    InputValue* val = FindInputValue(name);
    return (nullptr != val) ? val->GetValue() : 0.0f;
}

InputAxis* InputMap::AddInputAxis(std::string const &name)
{
    InputAxis* axis = nullptr;

    // If don't find it, create it.
    auto it = m_axies.find(name);
    if (m_axies.find(name) == m_axies.end())
    {
        axis = new InputAxis(this);
        m_axies[name] = axis;
    }
    else
    {
        axis = it->second;
    }

    return axis;
}

InputAxis* InputMap::AddInputAxis(const std::string& name, InputValue* positiveInput, InputValue* negativeInput)
{
    InputAxis* axis = AddInputAxis(name);
    axis->AddMapping(positiveInput, negativeInput);
    return axis;
}

InputAxis* InputMap::FindInputAxis(std::string const &name)
{
    // If we don't find it, create it.
    auto it = m_axies.find(name);
    if (m_axies.find(name) == m_axies.end())
    {
        return nullptr;
    }

    return it->second;
}

Vector2 InputMap::GetVector2(const std::string& xName, const std::string& yName)
{
    InputAxis* x = FindInputAxis(xName);
    InputAxis* y = FindInputAxis(yName);
    return Vector2(x->GetValue(), y->GetValue());
}
