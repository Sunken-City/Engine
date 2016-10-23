#pragma once
#include "Engine/Input/InputDevices/InputDevice.hpp"

//-----------------------------------------------------------------------------------
class KeyboardInputDevice : InputDevice
{
public:
    KeyboardInputDevice() {};
    ~KeyboardInputDevice() {};

    virtual void Update(float) override {}

    void SetKeyValue(unsigned char vkeyCode, bool isDown);
    InputValue* FindValue(unsigned char vkeyCode);

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int NUM_KEYS = 256;
    InputValue m_keys[NUM_KEYS];
};