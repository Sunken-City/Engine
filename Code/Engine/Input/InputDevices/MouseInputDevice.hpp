#pragma once
#include "Engine/Input/InputDevices/InputDevice.hpp"

//-----------------------------------------------------------------------------------
class MouseInputDevice : InputDevice
{
public:
    MouseInputDevice() { };
    ~MouseInputDevice() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float) override {}
    void SetButtonValue(unsigned char mouseButton, bool isDown);
    InputValue* FindButtonValue(unsigned char mouseButton);
    void SetDelta(const Vector2Int& cursorDelta);

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static void HideMouseCursor();
    static void ShowMouseCursor();
    static void LockMouseCursor();
    static void UnlockMouseCursor();
    static void CaptureMouseCursor();
    static void ReleaseMouseCursor();

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int NUM_AXIES = 2;
    static const int X_DELTA = 0;
    static const int Y_DELTA = 1;

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static bool m_isCursorVisible;
    static bool m_captureCursor;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputValue m_buttons[InputSystem::NUM_MOUSE_BUTTONS];
    InputVector2 m_deltaPosition;
};