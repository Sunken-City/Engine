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
