#pragma once
#include "Engine/Core/Event.hpp"

class InputMap;
class Vector2;

class InputBase
{
public:
    InputBase(InputMap* o) : m_owner(o) {}

    InputMap* m_owner;
};

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
    void SetValue(const float value);
    void OnChanged(const InputValue* v);
    void AddMapping(InputValue* v);

    float m_previousValue;
    float m_currentValue;
    Event<const InputValue*> m_OnChange;
    Event<const InputValue*> m_OnPress;
    Event<const InputValue*> m_OnRelease;
};

class InputAxis : public InputBase
{
public:
    InputAxis()
        : InputBase(nullptr)
        , m_negativeValue(nullptr)
        , m_positiveValue(nullptr)
    {

    }

    InputAxis(InputMap* owner)
        : InputBase(owner)
        , m_negativeValue(nullptr)
        , m_positiveValue(nullptr)
    {

    }

    void AddMapping(InputValue* pos, InputValue* neg);
    void OnValuesChanged(const InputValue*);
    void SetValue(float positiveValue, float negativeValue);
    float GetValue() const;
    bool HasChanged(float positiveValue, float negativeValue);

    InputValue* m_negativeValue;
    InputValue* m_positiveValue;
    Event<const InputValue*> m_OnChange;
};


// I also have CInputVector2, CInputVector3
// A raw thumbstick generates a CInputVector2
// class InputVector2 : InputBase
// {
// public:
//     InputVector2()
//         : InputBase(nullptr)
//         , x(nullptr)
//         , y(nullptr)
//     {
// 
//     }
// 
//     InputVector2(InputMap* owner)
//         : InputBase(owner)
//         , x(owner)
//         , y(owner)
//     {
// 
//     }
// 
//     void AddMapping(InputAxis* x, InputAxis* y);
//     Vector2 GetValue() const;
// 
//     InputAxis x;
//     InputAxis y;
//     Event<const InputValue*> m_OnChange;
// };
