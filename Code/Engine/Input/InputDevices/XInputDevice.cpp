#include "Engine/Input/InputDevices/XInputDevice.hpp"

//-----------------------------------------------------------------------------------
XInputDevice::XInputDevice(XInputController* controller) : m_controller(controller)
{
}

//-----------------------------------------------------------------------------------
void XInputDevice::Update(float)
{
    if (!m_controller->IsConnected())
    {
        return;
    }

    int numButtons = static_cast<int>(XboxButton::NUM_XBOX_BUTTONS);
    for (int button = 0; button < numButtons; ++button)
    {
        bool buttonIsPressed = m_controller->IsPressed(static_cast<XboxButton>(button));
        m_buttons->SetValue(buttonIsPressed ? 1.0f : 0.0f);
    }

    m_leftTrigger.SetValue(static_cast<float>(m_controller->GetLeftTrigger()) / 255.0f);
    m_rightTrigger.SetValue(static_cast<float>(m_controller->GetRightTrigger()) / 255.0f);
    m_leftStick.SetValue(m_controller->GetLeftStickPosition());
    m_rightStick.SetValue(m_controller->GetRightStickPosition());
}

//-----------------------------------------------------------------------------------
InputValue* XInputDevice::FindButton(XboxButton button)
{
    return &m_buttons[static_cast<int>(button)];
}
