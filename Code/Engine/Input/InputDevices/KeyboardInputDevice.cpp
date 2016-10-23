#include "Engine/Input/InputDevices/KeyboardInputDevice.hpp"

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
