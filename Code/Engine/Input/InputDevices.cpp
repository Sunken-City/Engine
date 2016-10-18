#include "Engine/Input/InputDevices.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "../Math/Vector2.hpp"
#include "Logging.hpp"

//-----------------------------------------------------------------------------------
void KeyboardInputDevice::SetKeyValue(unsigned char vkeyCode, bool isDown)
{
    m_keys[vkeyCode].SetValue(isDown ? 1.0f : 0.0f);
}

//-----------------------------------------------------------------------------------
InputValue* KeyboardInputDevice::FindValue(unsigned char vkeyCode)
{
    return &(m_keys[vkeyCode]);
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::SetButtonValue(unsigned char mouseButton, bool isDown)
{
    m_buttons[mouseButton].SetValue(isDown ? 1.0f : 0.0f);
}

//-----------------------------------------------------------------------------------
InputValue* MouseInputDevice::FindButtonValue(unsigned char mouseButton)
{
    return &(m_buttons[mouseButton]);
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::SetDelta(const Vector2Int& cursorDelta)
{
    m_deltaPosition.SetValue(Vector2(cursorDelta.x, -cursorDelta.y).GetNorm());
}
