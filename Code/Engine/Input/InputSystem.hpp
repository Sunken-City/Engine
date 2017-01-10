#pragma once
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Core/Events/Event.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class XInputController;
class KeyboardInputDevice;
class MouseInputDevice;
class XInputDevice;
enum class XboxButton;

class InputSystem
{
public:
    //ENUMS////////////////////////////////////////////////////////////////////////
    enum ExtraKeys
    {
        BACKSPACE = 0x08, //VK_BACK
        TAB = 0x09, //VK_TAB
        ENTER = 0x0D, //VK_RETURN
        SHIFT = 0x10, //VK_SHIFT
        ESC = 0x1B, //VK_ESCAPE
        PAGE_UP = 0x21, //VK_PRIOR
        PAGE_DOWN = 0x22, //VK_NEXT
        END = 0x23, //VK_END
        HOME = 0x24, //VK_HOME
        LEFT = 0x25, //VK_LEFT
        UP = 0x26, //VK_UP
        RIGHT = 0x27, //VK_RIGHT
        DOWN = 0x28, //VK_DOWN
        DEL = 0x2E, //VK_DELETE
        F5 = 0x74, //VK_F5
        F6 = 0x75, //VK_F6
        COMMA = 0xBC, //VK_OEM_COMMA
        PERIOD = 0xBE, //VK_OEM_PERIOD
        TILDE = 0xC0, //VK_OEM_3
        NUM_EXTRA_KEYS
    };

    enum MouseButton
    {
        LEFT_MOUSE_BUTTON = 0,
        RIGHT_MOUSE_BUTTON,
        MIDDLE_MOUSE_BUTTON,
        NUM_BUTTONS

    };

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    InputSystem(void* hWnd, int maximumNumberOfControllers = 0);
    ~InputSystem();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Update(float deltaTime);
    void AdvanceFrameNumber();

    //SETTERS//////////////////////////////////////////////////////////////////////////
    void SetCursorPosition(Vector2Int newPosition);
    void SetMouseWheelStatus(short deltaMouseWheel);
    void SetKeyDownStatus(unsigned char keyCode, bool isDown);
    void SetMouseDownStatus(unsigned char mouseButton, bool isNowDown);

    //QUERIES//////////////////////////////////////////////////////////////////////////
    bool IsKeyDown(unsigned char keyCode);
    bool WasKeyJustPressed(unsigned char keyCode);
    bool IsMouseButtonDown(unsigned char mouseButtonCode);
    bool WasMouseButtonJustPressed(unsigned char mouseButtonCode);
    bool WasMouseButtonJustReleased(unsigned char mouseButtonCode);
    bool IsScrolling();
    bool HasFocus();

    //GETTERS//////////////////////////////////////////////////////////////////////////
    int GetScrollAmountThisFrame();
    Vector2Int GetDeltaMouse();
    Vector2Int GetMousePos();
    void SetLastPressedChar(unsigned char asKey);
    char GetLastPressedChar();
    bool WasButtonJustPressed(XboxButton button, int controllerIndex = -1);

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int NUM_KEYS = 256;
    static const int NUM_MOUSE_BUTTONS = 3;
    static const int ABSOLUTE_MAX_NUM_CONTROLLERS = 4;

    //VARIABLES//////////////////////////////////////////////////////////////////////////
    static InputSystem* instance;
    XInputController* m_controllers[ABSOLUTE_MAX_NUM_CONTROLLERS];
    KeyboardInputDevice* m_keyboardDevice;
    MouseInputDevice* m_mouseDevice;
    XInputDevice* m_xInputDevices[ABSOLUTE_MAX_NUM_CONTROLLERS];
    Event<float> m_OnUpdate;
    
private:
    //CONSTANTS//////////////////////////////////////////////////////////////////////////
#pragma todo("These need to not be hard-coded, we're gonna have a bad time")
    static const int SNAP_BACK_X = 800;
    static const int SNAP_BACK_Y = 600;
    
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    bool m_isKeyDown[NUM_KEYS];
    bool m_isMouseDown[NUM_MOUSE_BUTTONS];
    bool m_isCursorVisible;
    bool m_hasFocus;
    bool m_captureCursor;
    bool m_isScrolling;
    int m_linesScrolled;
    int m_frameNumberKeyLastChanged[NUM_KEYS];
    int m_frameNumberMouseButtonLastChanged[NUM_MOUSE_BUTTONS];
    int m_frameCounter;
    char m_lastPressedChar;
    Vector2Int m_cursorDelta;
    Vector2Int m_cursorPosition;
    void* m_hWnd;

public:
    int m_maximumNumControllers;
};
