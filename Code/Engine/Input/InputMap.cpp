#include "Engine/Input/InputMap.hpp"
#include "Engine/Input/InputValues.hpp"

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

InputValue* InputMap::AddInputValue(std::string const &name, InputValue *other)
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

void InputMap::AddInputAxis(const std::string& name, InputValue* positiveInput, InputValue* negativeInput)
{
    throw std::logic_error("The method or operation is not implemented.");
}
