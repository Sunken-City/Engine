#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"
#include <algorithm>

//--------------------------------------------------------------
void InputValue::SetValue(const float value)
{
    if (value != m_currentValue)
    {
        m_previousValue = m_currentValue;
        m_currentValue = value;
        m_onChange.Trigger(this);

        if (WasJustReleased())
        {
            m_onRelease.Trigger(this);
        }
        else if (WasJustPressed())
        {
            m_onPress.Trigger(this);
        }
    }
}

//--------------------------------------------------------------
void InputValue::OnChanged(const InputValue* value)
{
    SetValue(value->GetValue());
}

//--------------------------------------------------------------
void VirtualInputValue::AddMapping(InputValue* value)
{
    value->m_onChange.RegisterMethod(this, &VirtualInputValue::OnValuesChanged);
#pragma todo("Check for dupes first")
    m_watchedValues.push_back(value);
}

//-----------------------------------------------------------------------------------
void VirtualInputValue::OnValuesChanged(const InputValue*)
{
    float value = m_watchedValues[0]->GetValue();
    
    for (int i = 1; i < m_watchedValues.size(); ++i)
    {
        if (m_chordResolutionMode == RESOLVE_MAXS)
        {
            value = std::max(value, m_watchedValues[i]->GetValue());
        }
        else
        {
            value = std::min(value, m_watchedValues[i]->GetValue());
        }
    }

    SetValue(value);
}

//--------------------------------------------------------------
void InputAxis::OnValuesChanged(const InputValue*)
{
    SetValue(m_positiveValue.GetValue() - m_negativeValue.GetValue());
}

//-----------------------------------------------------------------------------------
void InputVector2::SetValue(const Vector2& inputValue)
{
    if (inputValue != m_currentValue)
    {
        m_currentValue = inputValue;
        m_xAxis.SetValue(inputValue.x);
        m_yAxis.SetValue(inputValue.y);
        m_onChange.Trigger(this);
    }
}