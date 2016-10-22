#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"

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
void InputValue::OnChanged(const InputValue* v)
{
    SetValue(v->GetValue());
}

//--------------------------------------------------------------
void InputValue::AddMapping(InputValue* v)
{
    v->m_onChange.RegisterMethod(this, &InputValue::OnChanged);
}

//--------------------------------------------------------------
void InputAxis::AddMapping(InputValue* pos, InputValue* neg)
{
    pos->m_onChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
    neg->m_onChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
    m_positiveValue = pos;
    m_negativeValue = neg;
}

//--------------------------------------------------------------
void InputAxis::OnValuesChanged(const InputValue*)
{
    SetValue(m_positiveValue->GetValue(), m_negativeValue->GetValue());
}

//--------------------------------------------------------------
void InputAxis::SetValue(float positiveValue, float negativeValue)
{
    // I would like this not to do anything if you're setting it to the same value it already is.
    if (HasChanged(positiveValue, negativeValue))
    {
        m_positiveValue->SetValue(positiveValue);
        m_negativeValue->SetValue(negativeValue);
    }
    m_OnChange.Trigger(nullptr);
}

//--------------------------------------------------------------
float InputAxis::GetValue() const
{
    return m_positiveValue->GetValue() - m_negativeValue->GetValue();
}

//--------------------------------------------------------------
bool InputAxis::HasChanged(float positiveValue, float negativeValue)
{
    return (m_positiveValue->m_currentValue != positiveValue || m_negativeValue->m_currentValue != negativeValue);
}

//-----------------------------------------------------------------------------------
void InputVector2::SetValue(const Vector2& inputValue)
{
    m_currentValue = inputValue;
    SetAxis(inputValue.x, m_xPos, m_xNeg);
    SetAxis(inputValue.y, m_yPos, m_yNeg);
}

//-----------------------------------------------------------------------------------
void InputVector2::SetAxis(float newVal, InputValue* pos, InputValue* neg)
{
    //Set the negative value if negative, positive value if positive
    if (newVal < 0.0f)
    {
        if ((pos->m_currentValue != 0.0f || neg->m_currentValue != -newVal))
        {
            pos->SetValue(0.0f);
            neg->SetValue(-newVal);
            pos->m_onChange.Trigger(nullptr);
            neg->m_onChange.Trigger(nullptr);
        }
    }
    else
    {
        if ((pos->m_currentValue != newVal || neg->m_currentValue != 0.0f))
        {
            pos->SetValue(newVal);
            neg->SetValue(0.0f);
            pos->m_onChange.Trigger(nullptr);
            neg->m_onChange.Trigger(nullptr);
        }
    }
}
