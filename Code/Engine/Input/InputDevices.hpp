#pragma once
#include "Engine/Input/InputValues.hpp"

// Input Devices manage our raw inputs - our lowest level - tracks all hardware inputs
// (Axises, Values, what have you)
class InputDevice
{
public:
    virtual void Update(float deltaSeconds) = 0;
};

class KeyboardInputDevice : InputDevice
{
public:
    KeyboardInputDevice();
    ~KeyboardInputDevice();

    virtual void Update(float) override {}

    void SetKeyValue(unsigned char vkeyCode, bool isDown);
    InputValue* FindValue(unsigned char vkeyCode);

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int NUM_KEYS = 256;
    InputValue keys[NUM_KEYS];
};