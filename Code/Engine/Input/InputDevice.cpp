#include "Engine/Input/InputDevice.hpp"

void InputDeviceKeyboard::SetKeyValue(unsigned char vkeyCode, bool isDown)
{
    keys[vkeyCode].SetValue(isDown ? 1.0f : 0.0f);
}

InputValue* InputDeviceKeyboard::FindValue(unsigned char vkeyCode)
{
    return &(keys[vkeyCode]);
}
