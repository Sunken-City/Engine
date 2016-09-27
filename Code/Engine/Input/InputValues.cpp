#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"

//--------------------------------------------------------------
void InputValue::SetValue(const float value)
{
    if (value != m_currentValue)
    {
        m_previousValue = m_currentValue;
        m_currentValue = value;
        m_OnChange.Trigger(this);

        if (IsUp() && WasDown())
        {
            m_OnRelease.Trigger(this);
        }
        else if (IsDown() && WasUp())
        {
            m_OnPress.Trigger(this);
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
    v->m_OnChange.RegisterMethod(this, &InputValue::OnChanged);
}

//--------------------------------------------------------------
void InputAxis::AddMapping(InputValue* pos, InputValue* neg)
{
    pos->m_OnChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
    neg->m_OnChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
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

// --------------------------------------------------------------
// void InputVector2::AddMapping(InputAxis* x, InputAxis* y)
// {
//     x->m_OnChange.RegisterMethod(this, &InputVector2::OnValuesChanged);
//     y->m_OnChange.RegisterMethod(this, &InputVector2::OnValuesChanged);
// }
// 
// --------------------------------------------------------------
// void InputVector2::OnValuesChanged(const InputValue*)
// {
//     SetValue(x.GetValue(), y.GetValue());
// }
// 
// --------------------------------------------------------------
// void InputVector2::SetValue(const float xVal, const float yVal)
// {
//     // I would like this not to do anything if you're setting it to the same value it already is.
//     if (x.GetValue() != xVal || y.GetValue() != yVal)
//     {
//         x.SetValue(xVal)
//         m_positiveValue.SetValue(positiveValue);
//         m_negativeValue.SetValue(negativeValue);
//     }
//     m_OnChange.Trigger(nullptr);
// }
// 
// --------------------------------------------------------------
// Vector2 InputVector2::GetValue() const
// {
//     return Vector2(x.GetValue(), y.GetValue());
// }
