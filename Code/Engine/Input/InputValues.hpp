#pragma once
#include "Engine/Core/Event.hpp"
#include "../Math/Vector2.hpp"

class InputMap;
class Vector2;

//-----------------------------------------------------------------------------------
class InputBase
{
public:
    InputBase(InputMap* o) : m_owner(o) {}

    InputMap* m_owner;
};

//-----------------------------------------------------------------------------------
class InputValue : public InputBase
{
public:
    InputValue()
        : InputBase(nullptr),
        m_currentValue(0.0f),
        m_previousValue(0.0f) 
    {

    }

    InputValue(InputMap* owner)
        : InputBase(owner),
        m_currentValue(0.0f),
        m_previousValue(0.0f) 
    {

    }

    inline float GetValue() const { return m_currentValue; }
    inline bool IsDown() const { return (m_currentValue == 1.0f); }
    inline bool IsUp() const { return (m_currentValue == 0.0f); }
    inline bool WasDown() const { return (m_previousValue == 1.0f); }
    inline bool WasUp() const { return (m_previousValue == 0.0f); }
    inline bool WasJustReleased() const { return (m_previousValue == 1.0f && m_currentValue == 0.0f); }
    inline bool WasJustPressed() const { return (m_previousValue == 0.0f && m_currentValue == 1.0f); }
    void SetValue(const float value);
    void OnChanged(const InputValue* v);
    void AddMapping(InputValue* v);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    float m_previousValue;
    float m_currentValue;
    Event<const InputValue*> m_OnChange;
    Event<const InputValue*> m_OnPress;
    Event<const InputValue*> m_OnRelease;
};

//-----------------------------------------------------------------------------------
class InputAxis : public InputBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputAxis()
        : InputBase(nullptr)
        , m_negativeValue(nullptr)
        , m_positiveValue(nullptr)
    {}

    InputAxis(InputMap* owner)
        : InputBase(owner)
        , m_negativeValue(nullptr)
        , m_positiveValue(nullptr)
    {}

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void AddMapping(InputValue* pos, InputValue* neg);
    void OnValuesChanged(const InputValue*);
    void SetValue(float positiveValue, float negativeValue);
    float GetValue() const;
    bool HasChanged(float positiveValue, float negativeValue);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputValue* m_negativeValue;
    InputValue* m_positiveValue;
    Event<const InputValue*> m_OnChange;
};

//-----------------------------------------------------------------------------------
//Since this system doesn't handle joysticks well, this is a wrapper around 4 fake key inputs that make up a vector joystick.
//An input axis can get the "key values" from this input type, while this just takes a vector2 and determines what a virtual joystick would be pressing via 4 keys to make that happen
class InputVector2 : public InputBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputVector2()
        : InputBase(nullptr)
        , m_xPos(new InputValue(nullptr))
        , m_xNeg(new InputValue(nullptr))
        , m_yPos(new InputValue(nullptr))
        , m_yNeg(new InputValue(nullptr))
    {}

    InputVector2(InputMap* owner)
        : InputBase(owner)
        , m_xPos(new InputValue(owner))
        , m_xNeg(new InputValue(owner))
        , m_yPos(new InputValue(owner))
        , m_yNeg(new InputValue(owner))
    {}

    ~InputVector2()
    {
        delete m_xPos;
        delete m_xNeg;
        delete m_yPos;
        delete m_yNeg;
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetValue(const Vector2& inputValue);
    Vector2 GetValue() const { return m_currentValue; };
    void SetAxis(float newVal, InputValue* pos, InputValue* neg);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputValue* m_xPos;
    InputValue* m_xNeg;
    InputValue* m_yPos;
    InputValue* m_yNeg;
    Vector2 m_currentValue;
};