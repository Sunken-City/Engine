#pragma once
#include "Engine/Input/InputDevices/InputDevice.hpp"
#include "Engine/Input/XInputController.hpp"

class XInputDevice : public InputDevice
{
public:
    ////CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    XInputDevice(XInputController* controller);
    ~XInputDevice() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float) override;

    InputValue* FindButton(XboxButton button);
    inline InputValue* GetLeftTrigger() { return &m_leftTrigger; };
    inline InputValue* GetRightTrigger() { return &m_rightTrigger; };
    inline InputValue* GetLeftStickMagnitude() { return &m_leftStickMagnitude; };
    inline InputValue* GetRightStickMagnitude() { return &m_rightStickMagnitude; };
    inline InputVector2* GetLeftStick() { return &m_leftStick; };
    inline InputVector2* GetRightStick() { return &m_rightStick; };
    inline InputVector2* GetInvertedLeftStick() { return &m_invertedLeftStick; };
    inline InputVector2* GetInvertedRightStick() { return &m_invertedRightStick; };

private:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    XInputController* m_controller;
    InputValue m_buttons[static_cast<int>(XboxButton::NUM_XBOX_BUTTONS)];
    InputValue m_leftTrigger;
    InputValue m_rightTrigger;
    InputValue m_leftStickMagnitude;
    InputValue m_rightStickMagnitude;
    InputVector2 m_leftStick;
    InputVector2 m_rightStick;
    InputVector2 m_invertedLeftStick;
    InputVector2 m_invertedRightStick;
};