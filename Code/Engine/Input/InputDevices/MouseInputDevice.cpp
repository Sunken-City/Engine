#include "Engine/Input/InputDevices/MouseInputDevice.hpp"

#ifdef WIN32
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

bool MouseInputDevice::m_isCursorVisible = true;
bool MouseInputDevice::m_captureCursor = false;

//-----------------------------------------------------------------------------------
void MouseInputDevice::SetButtonValue(unsigned char mouseButton, bool isDown)
{
    m_buttons[mouseButton].SetValue(isDown ? 1.0f : 0.0f);
}

//-----------------------------------------------------------------------------------
InputValue* MouseInputDevice::FindButtonValue(unsigned char mouseButton)
{
    return &(m_buttons[mouseButton]);
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::SetDelta(const Vector2Int& cursorDelta)
{
    m_deltaPosition.SetValue(Vector2((float)cursorDelta.x, (float)-cursorDelta.y).GetNorm());
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::HideMouseCursor()
{
    ShowCursor(FALSE);
    m_isCursorVisible = false;
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::ShowMouseCursor()
{
    ShowCursor(TRUE);
    m_isCursorVisible = true;
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::LockMouseCursor()
{
    m_captureCursor = true;
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::UnlockMouseCursor()
{
    m_captureCursor = false;
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::CaptureMouseCursor()
{
    HideMouseCursor();
    LockMouseCursor();
}

//-----------------------------------------------------------------------------------
void MouseInputDevice::ReleaseMouseCursor()
{
    ShowMouseCursor();
    UnlockMouseCursor();
}
