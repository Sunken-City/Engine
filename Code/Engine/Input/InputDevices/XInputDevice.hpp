#pragma once
#include "Engine/Input/InputDevices/InputDevice.hpp"
#include "Engine/Input/XInputController.hpp"

class XInputDevice : public InputDevice
{
public:
    XInputDevice(XInputController* controller);
    ~XInputDevice() {};

    virtual void Update(float) override;

    InputValue* FindButton(XboxButton button);
    inline InputValue* GetLeftTrigger() { return &m_leftTrigger; };
    inline InputValue* GetRightTrigger() { return &m_rightTrigger; };
    inline InputVector2* GetLeftStick() { return &m_leftStick; };
    inline InputVector2* GetRightStick() { return &m_rightStick; };

private:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    XInputController* m_controller;
    InputValue m_buttons[static_cast<int>(XboxButton::NUM_XBOX_BUTTONS)];
    InputValue m_leftTrigger;
    InputValue m_rightTrigger;
    InputVector2 m_leftStick;
    InputVector2 m_rightStick;
};