#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Input/InputDevices/KeyboardInputDevice.hpp"
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"
#include "InputDevices/XInputDevice.hpp"
#include "../Core/ProfilingUtils.h"

InputSystem* InputSystem::instance = nullptr;

//-----------------------------------------------------------------------------------
InputSystem::InputSystem(void* hWnd, int maximumNumberOfControllers /*= 0*/)
: m_frameCounter(0)
, m_cursorDelta(0, 0)
, m_cursorPosition(0, 0)
, m_isCursorVisible(false)
, m_hasFocus(false)
, m_hWnd(hWnd)
, m_isScrolling(false)
, m_linesScrolled(0)
, m_maximumNumControllers(maximumNumberOfControllers)
, m_lastPressedChar(0x00) //NULL character
, m_captureCursor(false)
, m_keyboardDevice(new KeyboardInputDevice())
, m_mouseDevice(new MouseInputDevice())
{
    //Only initialize the number of controllers we need for the game.
    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        m_controllers[i] = new XInputController(i);
        m_xInputDevices[i] = new XInputDevice(m_controllers[i]);
    }

    m_OnUpdate.RegisterMethod(m_keyboardDevice, &KeyboardInputDevice::Update);
    m_OnUpdate.RegisterMethod(m_mouseDevice, &MouseInputDevice::Update);
    for (int i = 0; i < m_maximumNumControllers; ++i)
    {
        m_OnUpdate.RegisterMethod(m_xInputDevices[i], &XInputDevice::Update);
    }

    //Initialize all keys to up
    for (int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex)
    {
        m_isKeyDown[keyIndex] = false;
        m_frameNumberKeyLastChanged[keyIndex] = m_frameCounter;
    }

    //Initialize all mouse buttons to up
    for (int mouseButtonIndex = 0; mouseButtonIndex < NUM_MOUSE_BUTTONS; ++mouseButtonIndex)
    {
        m_isMouseDown[mouseButtonIndex] = false;
        m_frameNumberMouseButtonLastChanged[mouseButtonIndex] = m_frameCounter;
    }
    ShowCursor(TRUE);
}

//-----------------------------------------------------------------------------------
InputSystem::~InputSystem()
{
    InputSystem::instance->m_OnUpdate.UnregisterMethod(m_keyboardDevice, &KeyboardInputDevice::Update);
    InputSystem::instance->m_OnUpdate.UnregisterMethod(m_mouseDevice, &MouseInputDevice::Update);
    for (int i = 0; i < m_maximumNumControllers; ++i)
    {
        m_OnUpdate.UnregisterMethod(m_xInputDevices[i], &XInputDevice::Update);
    }

    delete m_keyboardDevice;
    delete m_mouseDevice;

    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        delete m_controllers[i];
        delete m_xInputDevices[i];
    }
}

//-----------------------------------------------------------------------------------
void InputSystem::EnablePollingForXInputConnections()
{
    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        XInputController* controller = m_controllers[i];
        controller->SetControllerNumber(i);
    }
}

//-----------------------------------------------------------------------------------
void InputSystem::DisablePollingForXInputConnections()
{
    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        XInputController* controller = m_controllers[i];
        if (!controller->IsConnected())
        {
            controller->SetControllerNumber(XInputController::INVALID_CONTROLLER_NUMBER);
        }
    }
}


//-----------------------------------------------------------------------------------
void InputSystem::Update(float deltaSeconds)
{
    //Controller Updates
    ProfilingSystem::instance->PushSample("ControllerUpdates");
    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        ProfilingSystem::instance->PushSample("ControllerUpdate");
        m_controllers[i]->Update(deltaSeconds);
        ProfilingSystem::instance->PopSample("ControllerUpdate");
    }
    ProfilingSystem::instance->PopSample("ControllerUpdates");

    //Mouse Updates
    HWND hWnd = static_cast<HWND>(m_hWnd);
    m_hasFocus = hWnd == GetFocus();
    if (m_hasFocus)
    {
        POINT cursorPos;
        BOOL success = GetCursorPos(&cursorPos);
        if (success)
        {
            if (ScreenToClient(hWnd, &cursorPos))
            {
                m_cursorPosition = Vector2Int(cursorPos.x, cursorPos.y);
                m_cursorDelta.x = cursorPos.x - SNAP_BACK_X;
                m_cursorDelta.y = cursorPos.y - SNAP_BACK_Y;
                m_mouseDevice->SetDelta(m_cursorDelta);
                if (!m_isCursorVisible && m_captureCursor)
                {
                    SetCursorPos(SNAP_BACK_X, SNAP_BACK_Y);
                }
            }

        }
    }

    m_OnUpdate.Trigger(deltaSeconds);
}

//-----------------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned char keyCode)
{
    return m_isKeyDown[keyCode];
}

//-----------------------------------------------------------------------------------
bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
    return (m_isKeyDown[keyCode] && (m_frameNumberKeyLastChanged[keyCode] == m_frameCounter));
}

//-----------------------------------------------------------------------------------
bool InputSystem::IsMouseButtonDown(unsigned char mouseButtonCode)
{
    return m_isMouseDown[mouseButtonCode];
}

//-----------------------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustPressed(unsigned char mouseButtonCode)
{
    return (m_isMouseDown[mouseButtonCode] && (m_frameNumberMouseButtonLastChanged[mouseButtonCode] == m_frameCounter));
}

//-----------------------------------------------------------------------------------
bool InputSystem::WasMouseButtonJustReleased(unsigned char mouseButtonCode)
{
    return (!m_isMouseDown[mouseButtonCode] && (m_frameNumberMouseButtonLastChanged[mouseButtonCode] == m_frameCounter));
}

//-----------------------------------------------------------------------------------
bool InputSystem::IsScrolling()
{
    return m_isScrolling;
}

//-----------------------------------------------------------------------------------
int InputSystem::GetScrollAmountThisFrame()
{
    return m_linesScrolled;
}

//-----------------------------------------------------------------------------------
Vector2Int InputSystem::GetDeltaMouse()
{
    return m_cursorDelta;
}

//-----------------------------------------------------------------------------------
Vector2Int InputSystem::GetMousePos()
{
    return m_cursorPosition;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetLastPressedChar(unsigned char asKey)
{
    m_lastPressedChar = asKey;
}

//-----------------------------------------------------------------------------------
char InputSystem::GetLastPressedChar()
{
    return m_lastPressedChar;
}

//-----------------------------------------------------------------------------------
bool InputSystem::WasButtonJustPressed(XboxButton button, int controllerIndex)
{
    if (controllerIndex > -1 && controllerIndex < XInputController::MAX_CONTROLLERS)
    {
        return m_controllers[controllerIndex] && m_controllers[controllerIndex]->JustPressed(button);
    }
    else
    {
        bool wasButtonPressedOnAnyController = false;

        for (int i = 0; i < XInputController::MAX_CONTROLLERS; ++i)
        {
            bool wasJustPressed = m_controllers[i] && m_controllers[i]->JustPressed(button);
            wasButtonPressedOnAnyController = wasButtonPressedOnAnyController || wasJustPressed;
        }

        return wasButtonPressedOnAnyController;
    }
}

//-----------------------------------------------------------------------------------
void InputSystem::SetCursorPosition(Vector2Int newPosition)
{
    SetCursorPos(newPosition.x, newPosition.y);
}

//-----------------------------------------------------------------------------------
bool InputSystem::HasFocus()
{
    return m_hasFocus;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetMouseWheelStatus(short deltaMouseWheel)
{
    int MOUSE_WHEEL_SCROLL_AMOUNT_PER_LINE = 120;
    m_isScrolling = true;
    m_linesScrolled = deltaMouseWheel / MOUSE_WHEEL_SCROLL_AMOUNT_PER_LINE;
}

//-----------------------------------------------------------------------------------
void InputSystem::AdvanceFrameNumber()
{
    ++m_frameCounter;
    m_isScrolling = false;
    m_linesScrolled = 0;
    m_lastPressedChar = 0x00;
}

//-----------------------------------------------------------------------------------
void InputSystem::ClearAndRecreateInputDevices()
{
    InputSystem::instance->m_OnUpdate.UnregisterMethod(m_keyboardDevice, &KeyboardInputDevice::Update);
    InputSystem::instance->m_OnUpdate.UnregisterMethod(m_mouseDevice, &MouseInputDevice::Update);
    for (int i = 0; i < m_maximumNumControllers; ++i)
    {
        m_OnUpdate.UnregisterMethod(m_xInputDevices[i], &XInputDevice::Update);
    }
    delete m_keyboardDevice;
    delete m_mouseDevice;

    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        delete m_controllers[i];
        delete m_xInputDevices[i];
    }



    m_keyboardDevice = new KeyboardInputDevice();
    m_mouseDevice = new MouseInputDevice();
    for (int i = 0; i < m_maximumNumControllers; i++)
    {
        m_controllers[i] = new XInputController(i);
        m_xInputDevices[i] = new XInputDevice(m_controllers[i]);
    }

    m_OnUpdate.RegisterMethod(m_keyboardDevice, &KeyboardInputDevice::Update);
    m_OnUpdate.RegisterMethod(m_mouseDevice, &MouseInputDevice::Update);
    for (int i = 0; i < m_maximumNumControllers; ++i)
    {
        m_OnUpdate.RegisterMethod(m_xInputDevices[i], &XInputDevice::Update);
    }
}

//-----------------------------------------------------------------------------------
void InputSystem::SetKeyDownStatus(unsigned char keyCode, bool isNowDown)
{
    m_keyboardDevice->SetKeyValue(keyCode, isNowDown);
    //If we are getting a keyboard repeat, ignore it when updating "just pressed" values.
    if (m_isKeyDown[keyCode] != isNowDown)
    {
        m_frameNumberKeyLastChanged[keyCode] = m_frameCounter;
    }
    m_isKeyDown[keyCode] = isNowDown;
}

//-----------------------------------------------------------------------------------
void InputSystem::SetMouseDownStatus(unsigned char mouseButton, bool isNowDown)
{
    m_mouseDevice->SetButtonValue(mouseButton, isNowDown);
    //If we are getting a keyboard repeat, ignore it when updating "just pressed" values.
    if (m_isMouseDown[mouseButton] != isNowDown)
    {
        m_frameNumberMouseButtonLastChanged[mouseButton] = m_frameCounter;
    }
    m_isMouseDown[mouseButton] = isNowDown;
}
