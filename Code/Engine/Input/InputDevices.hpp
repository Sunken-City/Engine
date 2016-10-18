#pragma once
#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Input/InputSystem.hpp"

// Input Devices manage our raw inputs - our lowest level - tracks all hardware inputs
// (Axises, Values, what have you)
class InputDevice
{
public:
    virtual void Update(float deltaSeconds) = 0;
};

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

//-----------------------------------------------------------------------------------
class MouseInputDevice : InputDevice
{
public:
    MouseInputDevice() { };
    ~MouseInputDevice() {};

    virtual void Update(float) override {}

    void SetButtonValue(unsigned char mouseButton, bool isDown);
    InputValue* FindButtonValue(unsigned char mouseButton);
    void SetDelta(const Vector2Int& cursorDelta);

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int NUM_AXIES = 2;
    static const int X_DELTA = 0;
    static const int Y_DELTA = 1;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputValue m_buttons[InputSystem::NUM_MOUSE_BUTTONS];
    InputVector2 m_deltaPosition;
};