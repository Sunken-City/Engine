#include "Engine/Input/InputDevices.hpp"
#include "Engine/Input/InputSystem.hpp"

KeyboardInputDevice::KeyboardInputDevice()
{

}

KeyboardInputDevice::~KeyboardInputDevice()
{
}

void KeyboardInputDevice::SetKeyValue(unsigned char vkeyCode, bool isDown)
{
    keys[vkeyCode].SetValue(isDown ? 1.0f : 0.0f);
}

InputValue* KeyboardInputDevice::FindValue(unsigned char vkeyCode)
{
    return &(keys[vkeyCode]);
}
