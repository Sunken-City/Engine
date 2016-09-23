#include "Engine/Input/InputValues.hpp"

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
    /* Example Usage
    CInputValue *w = keyboard->find_value( cKeyboard_W );
    CInputvalue *k8 = keyboard->find_value( cKeyboard_Keypad8 );

    CInputValue *up = gInputMap->add_value( "up" );
    up->add_mapping( w );
    up->add_mapping( k8 );
    */

    v->m_OnChange.RegisterMethod(this, &InputValue::OnChanged);
}

//--------------------------------------------------------------
void InputAxis::AddMapping(InputValue* pos, InputValue* neg)
{
    pos->m_OnChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
    neg->m_OnChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
}

//--------------------------------------------------------------
void InputAxis::OnValuesChanged(const InputValue*)
{
    SetValue(m_positiveValue.GetValue(), m_negativeValue.GetValue());
}

//--------------------------------------------------------------
void InputAxis::SetValue(float positiveValue, float negativeValue)
{
    // I would like this not to do anything if you're setting it to the same value it already is.
    if (m_positiveValue.m_currentValue != positiveValue && m_negativeValue.m_currentValue != negativeValue)
    {
        m_positiveValue.SetValue(positiveValue);
        m_negativeValue.SetValue(negativeValue);
    }
    m_OnChanged.Trigger(nullptr);
}

//--------------------------------------------------------------
float InputAxis::GetValue() const
{
    return m_positiveValue.GetValue() - m_negativeValue.GetValue();
}

