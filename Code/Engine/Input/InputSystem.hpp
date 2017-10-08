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
        CTRL = 0x11, //VK_CONTROL
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
        F7 = 0x76, //VK_F7
        F8 = 0x77, //VK_F8
        F9 = 0x78, //VK_F9
        F10 = 0x79, //VK_F10
        VOLUME_MUTE = 0xAD, //VK_VOLUME_MUTE
        VOLUME_DOWN = 0xAE, //VK_VOLUME_DOWN
        VOLUME_UP = 0xAF, //VK_VOLUME_UP
        NEXT_TRACK = 0xB0, //VK_MEDIA_NEXT_TRACK
        PREV_TRACK = 0xB1, //VK_MEDIA_PREV_TRACK
        STOP = 0xB2, //VK_MEDIA_STOP
        PLAY_PAUSE = 0xB3, //VK_MEDIA_PLAY_PAUSE
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

    //From https://msdn.microsoft.com/en-us/library/windows/desktop/ms648395(v=vs.85).aspx
    enum CursorType
    {
        OCR_APPSTARTING = 32650, //Standard arrow and small hourglass
        OCR_NORMAL = 32512, //Standard arrow
        OCR_CROSS = 32515, //Crosshair
        OCR_HAND = 32649, //Hand
        OCR_HELP = 32651, //Arrow and question mark
        OCR_IBEAM = 32513, //I - beam
        OCR_NO = 32648, //Slashed circle
        OCR_SIZEALL = 32646, //Four - pointed arrow pointing north, south, east, and west
        OCR_SIZENESW = 32643, //Double - pointed arrow pointing northeast and southwest
        OCR_SIZENS = 32645, //Double - pointed arrow pointing north and south
        OCR_SIZENWSE = 32642, //Double - pointed arrow pointing northwest and southeast
        OCR_SIZEWE = 32644, //Double - pointed arrow pointing west and east
        OCR_UP = 32516, //Vertical arrow
        OCR_WAIT = 32514, //Hourglass
    };

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    InputSystem(void* hWnd, int maximumNumberOfControllers = 0, int windowWidth = 1600, int windowHeight = 900);
    ~InputSystem();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Update(float deltaTime);
    void AdvanceFrameNumber();
    void ClearAndRecreateInputDevices();
    void EnablePollingForXInputConnections();
    void DisablePollingForXInputConnections();

    //SETTERS//////////////////////////////////////////////////////////////////////////
    void SetCursorPosition(Vector2Int newPosition);
    void SetMouseWheelStatus(short deltaMouseWheel);
    void SetKeyDownStatus(unsigned char keyCode, bool isDown);
    void SetMouseDownStatus(unsigned char mouseButton, bool isNowDown);
    void SetCursorType(CursorType cursorType); //Changes the windows cursor to one of the standard types

    //QUERIES//////////////////////////////////////////////////////////////////////////
    bool IsKeyDown(unsigned char keyCode);
    bool WasKeyJustPressed(unsigned char keyCode);
    bool IsMouseButtonDown(unsigned char mouseButtonCode);
    bool WasMouseButtonJustPressed(unsigned char mouseButtonCode);
    bool WasMouseButtonJustReleased(unsigned char mouseButtonCode);
    bool IsScrolling();
    bool HasFocus();
    inline unsigned int GetFrameNumber() { return m_frameCounter; };

    //GETTERS//////////////////////////////////////////////////////////////////////////
    int GetScrollDeltaThisFrame();
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
    
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    int m_mouseSnapBackX;
    int m_mouseSnapBackY;
    bool m_isKeyDown[NUM_KEYS];
    bool m_isMouseDown[NUM_MOUSE_BUTTONS];
    bool m_hasFocus;
    bool m_isScrolling;
    int m_linesScrolled;
    unsigned int m_frameNumberKeyLastChanged[NUM_KEYS];
    unsigned int m_frameNumberMouseButtonLastChanged[NUM_MOUSE_BUTTONS];
    unsigned int m_frameCounter;
    char m_lastPressedChar;
    Vector2Int m_cursorDelta;
    Vector2Int m_cursorPosition;
    void* m_hWnd;

public:
    int m_maximumNumControllers;
};
