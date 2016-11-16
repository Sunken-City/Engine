#pragma once
#include "Engine/Core/Events/Event.hpp"
#include "../Math/Vector2.hpp"

class InputMap;
class Vector2;

//-----------------------------------------------------------------------------------
enum ChordResolutionMode
{
    RESOLVE_MINS,
    RESOLVE_MAXS,
    RESOLVE_MAXS_ABSOLUTE,
    RESOLVE_MINS_ABSOLUTE
};

//-----------------------------------------------------------------------------------
class InputBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputBase(InputMap* o) : m_owner(o) {}

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputMap* m_owner;
};

//-----------------------------------------------------------------------------------
class InputValue : public InputBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputValue(InputMap* owner = nullptr)
        : InputBase(owner),
        m_currentValue(0.0f),
        m_previousValue(0.0f),
        m_deadzoneValue(0.15f)
    {

    }

    virtual ~InputValue() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    inline float GetValue() const { return m_currentValue; }
    inline bool IsDown() const { return (m_currentValue > 1.0f - m_deadzoneValue); }
    inline bool IsUp() const { return (m_currentValue < m_deadzoneValue); }
    inline bool DownLastFrame() const { return (m_previousValue > 1.0f - m_deadzoneValue); }
    inline bool UpLastFrame() const { return (m_previousValue < m_deadzoneValue); }
    inline bool WasJustReleased() const { return !IsDown() && DownLastFrame(); }
    inline bool WasJustPressed() const { return !IsUp() && UpLastFrame(); }
    void SetValue(const float value);
    void OnChanged(const InputValue* value);

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
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    VirtualInputValue(InputMap* owner = nullptr, ChordResolutionMode mode = RESOLVE_MAXS)
        : InputValue(owner)
        , m_chordResolutionMode(mode)
    {
        m_watchedValues.reserve(8);
    }

    virtual ~VirtualInputValue() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void AddMapping(InputValue* value);
    void RemoveMapping(InputValue*) {};
    void ClearMappings() {};
    void OnValuesChanged(const InputValue* value);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<InputValue*> m_watchedValues;
    ChordResolutionMode m_chordResolutionMode;
};

//-----------------------------------------------------------------------------------
class InputAxis : public VirtualInputValue
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputAxis(InputMap* owner = nullptr, ChordResolutionMode mode = RESOLVE_MAXS)
        : VirtualInputValue(owner, mode)
        , m_negativeValue(owner, mode)
        , m_positiveValue(owner, mode)
    {
        m_positiveValue.m_onChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
        m_negativeValue.m_onChange.RegisterMethod(this, &InputAxis::OnValuesChanged);
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void OnValuesChanged(const InputValue*);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    VirtualInputValue m_negativeValue;
    VirtualInputValue m_positiveValue;
};

//-----------------------------------------------------------------------------------
class InputVector2 : public InputBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputVector2(InputMap* owner = nullptr)
        : InputBase(owner)
        , m_currentValue(0.0f)
    {}

    ~InputVector2()
    {
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetValue(const Vector2& inputValue);
    Vector2 GetValue() const { return m_currentValue; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    InputAxis m_xAxis;
    InputAxis m_yAxis;
    Vector2 m_currentValue;
    Event<const InputVector2*> m_onChange;
};