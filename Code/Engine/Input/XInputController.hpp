#pragma once

#ifdef WIN32
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <Xinput.h> // include the Xinput API
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; using 9_1_0 explicitly

class Vector2;

//ENUMS//////////////////////////////////////////////////////////////////////////
enum class XboxButton
{
    DUP,
    DDOWN,
    DLEFT,
    DRIGHT,
    A,
    B,
    X,
    Y,
    LB,
    RB,
    LT,
    RT,
    START,
    BACK,
    HOME,
    numXboxButtons
};


class XInputController
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    XInputController();
    XInputController(int controllerNumber);
    
    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Update(float deltaTime);
    /*Left motor is the heavy-duty motor, creates large vibrations.
    * Right motor is for finer and more gentle vibrations. */
    void Vibrate(int leftMotorVibration, int rightMotorVibration);
    void VibrateForSeconds(float seconds, int leftMotorVibration, int rightMotorVibration);

    //QUERIES//////////////////////////////////////////////////////////////////////////
    bool IsPressed(XboxButton button);
    bool IsConnected();
    bool JustPressed(XboxButton button);
    bool IsRightStickPastDeadzone();
    bool IsLeftStickPastDeadzone();

    //GETTERS//////////////////////////////////////////////////////////////////////////
    int GetControllerNumber();
    //Returns the normalized position of the left stick (from 0 to 1)
    Vector2 GetLeftStickPosition();
    //Returns the normalized position of the right stick (from 0 to 1)
    Vector2 GetRightStickPosition();
    //Returns the angle in degrees of the right stick, where 0 is up
    float GetRightStickAngleDegrees();
    //Returns the angle in degrees of the left stick, where 0 is up
    float GetLeftStickAngleDegrees();
    unsigned char GetLeftTrigger();
    unsigned char GetRightTrigger();

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int MAX_CONTROLLERS = 4;
    static const int MAX_VIBRATE = 65535;
    static const int LARGE_VIBRATE = 48000;
    static const int MEDIUM_VIBRATE = 32000;
    static const int LOW_VIBRATE = 16000;
    static const int EXTREMELY_LOW_VIBRATE = 8000;
    static const int NO_VIBRATE = 0;
    static const float INNER_DEADZONE;
    static const float OUTER_DEADZONE;

private:
    static const int INVALID_CONTROLLER_NUMBER = -1;

    //HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
    bool IsPressed(XINPUT_STATE &state, XboxButton button);
    Vector2 CalculateNormalizedStickPosition(float x, float y);
    float CalculateStickAngleRadians(float x, float y);

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    XINPUT_STATE m_state;
    XINPUT_STATE m_previousState;
    int m_controllerNumber;
    bool m_isConnected;
    float m_secondsToVibrate;
};