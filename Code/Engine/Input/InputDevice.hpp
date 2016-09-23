#pragma once
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/InputValues.hpp"

// Input Devices manage our raw inputs - our lowest level - tracks all hardware inputs
// (Axises, Values, what have you)
class InputDevice
{
public:
    virtual void Update() = 0;
};

class InputDeviceKeyboard : InputDevice
{
public:
    virtual void Update() override {}

    void SetKeyValue(unsigned char vkeyCode, bool isDown);
    InputValue* FindValue(unsigned char vkeyCode);

    InputValue keys[InputSystem::NUM_KEYS];
};