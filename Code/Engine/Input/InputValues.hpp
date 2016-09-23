#pragma once
#include "Engine/Core/Event.hpp"

class InputMap;

class InputBase
{
public:
    InputBase(InputMap* o) : owner(o) {}

    InputMap* owner;
};

class InputValue : InputBase
{
public:
    InputValue(InputMap *owner)
        : InputBase(owner),
        m_currentValue(0.0f),
        m_previousValue(0.0f) {}

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

// class InputAxis : InputBase
// {
// };
// 
// // I also have CInputVector2, CInputVector3
// // A raw thumbstick generates a CInputVector2
// class InputVector2 : InputBase
// {
// public:
//     InputAxis x;
//     InputAxis y;
// 
//     void InputVector2
// 
//         vec2 get_value() const
//     {
//         return vec2(x.get_value(), y.get_value());
//     }
// };
