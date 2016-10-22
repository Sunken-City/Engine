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
        m_previousValue(0.0f),
        m_deadzoneValue(0.15f)
    {

    }

    InputValue(InputMap* owner)
        : InputBase(owner),
        m_currentValue(0.0f),
        m_previousValue(0.0f),
        m_deadzoneValue(0.15f)
    {

    }

    inline float GetValue() const { return m_currentValue; }
    inline bool IsDown() const { return (m_currentValue > 1.0f - m_deadzoneValue); }
    inline bool IsUp() const { return (m_currentValue < m_deadzoneValue); }
    inline bool WasDown() const { return (m_previousValue > 1.0f - m_deadzoneValue); }
    inline bool WasUp() const { return (m_previousValue < m_deadzoneValue); }
    inline bool WasJustReleased() const { return !IsDown() && WasDown(); }
    inline bool WasJustPressed() const { return !IsUp() && WasUp(); }
    void SetValue(const float value);
    void OnChanged(const InputValue* v);
    void AddMapping(InputValue* v);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    float m_previousValue;
    float m_currentValue;
    float m_deadzoneValue;
    Event<const InputValue*> m_onChange;
    Event<const InputValue*> m_onPress;
    Event<const InputValue*> m_onRelease;
};

//-----------------------------------------------------------------------------------
class VirtualInputValue : public InputValue
{

};

//-----------------------------------------------------------------------------------
class InputAxis : public InputValue
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputAxis()
        : InputValue()
        , m_negativeValue(nullptr)
        , m_positiveValue(nullptr)
    {}

    InputAxis(InputMap* owner)
        : InputValue(owner)
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
    VirtualInputValue m_negativeValue;
    VirtualInputValue m_positiveValue;
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
    {}

    InputVector2(InputMap* owner)
        : InputBase(owner)
    {}

    ~InputVector2()
    {
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetValue(const Vector2& inputValue);
    Vector2 GetValue() const { return m_currentValue; };
    void SetAxis(float newVal, InputValue* pos, InputValue* neg);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputAxis m_x;
    InputAxis m_y;
    Vector2 m_currentValue;
};