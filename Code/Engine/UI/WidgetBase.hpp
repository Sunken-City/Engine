#pragma once
#include <vector>
#include "Engine/Input/XMLUtils.hpp"
#include "Engine/Core/Events/NamedProperties.hpp"
#include "Engine/Renderer/AABB2.hpp"

class Matrix4x4;

//-----------------------------------------------------------------------------------
enum WidgetState
{
    ACTIVE_WIDGET_STATE,
    HIGHLIGHTED_WIDGET_STATE,
    PRESSED_WIDGET_STATE,
    DISABLED_WIDGET_STATE,
    HIDDEN_WIDGET_STATE,
    NUM_STATES,
    DEFAULT_STATE = NUM_STATES
};

//-----------------------------------------------------------------------------------
class WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    WidgetBase();
    virtual ~WidgetBase();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    template <typename T>
    void SetProperty(const std::string& propertyName, const T& value, WidgetState state = DEFAULT_STATE)
    {
        if (state == DEFAULT_STATE)
        {
            m_propertiesForAllStates.Set<T>(propertyName, value);
        }
        else
        {
            m_propertiesForState[state].Set<T>(propertyName, value);
        }
    }

    //-----------------------------------------------------------------------------------
    void SetProperty(const std::string& propertyName, const std::string& value, WidgetState state = DEFAULT_STATE)
    {
        if (state == DEFAULT_STATE)
        {
            m_propertiesForAllStates.Set(propertyName, value);
        }
        else
        {
            m_propertiesForState[state].Set(propertyName, value);
        }
    }

    //-----------------------------------------------------------------------------------
    template <typename T>
    T GetProperty(const std::string& propertyName) const
    {
        T returnValue;
        PropertyGetResult result = m_propertiesForState[m_currentState].Get<T>(propertyName, returnValue);
        if (result != PGR_SUCCESS)
        {
            returnValue = m_propertiesForAllStates.Get<T>(propertyName);
        }
        return returnValue;
    }

    //-----------------------------------------------------------------------------------
    std::string GetProperty(const std::string& propertyName) const
    {
        std::string returnValue;
        PropertyGetResult result = m_propertiesForState[m_currentState].Get<std::string>(propertyName, returnValue);
        if (result != PGR_SUCCESS)
        {
            returnValue = m_propertiesForAllStates.Get<std::string>(propertyName);
        }
        return returnValue;
    }

    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void AddChild(WidgetBase* child);
    virtual AABB2 GetBounds() { return m_bounds; };
    virtual AABB2 GetSmallestBoundsAroundChildren();
    virtual void RecalculateBounds() = 0;
    virtual void BuildFromXMLNode(XMLNode& node);
    virtual void OnClick();
    virtual WidgetBase* GetWidgetPointIsInside(const Vector2& point); //Is this inside you or any of your children?
    virtual void SetHighlighted() { m_previousState = m_currentState; m_currentState = HIGHLIGHTED_WIDGET_STATE; };
    virtual void UnsetHighlighted() { m_currentState = m_previousState; };
    virtual void SetPressed() { m_currentState = PRESSED_WIDGET_STATE; };
    virtual void UnsetPressed() { m_currentState = m_previousState; };
    inline bool IsHighlighted() const { return m_currentState == HIGHLIGHTED_WIDGET_STATE; };
    Vector2 GetParentOffsets() const;
    Matrix4x4 GetModelMatrix() const;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    mutable NamedProperties m_propertiesForAllStates;
    mutable NamedProperties m_propertiesForState[NUM_STATES];
    std::vector<WidgetBase*> m_children;
    std::string m_name;
    AABB2 m_bounds;
    WidgetBase* m_parent = nullptr;
    WidgetState m_previousState = ACTIVE_WIDGET_STATE;
    WidgetState m_currentState = ACTIVE_WIDGET_STATE;
    const bool m_isInteractive = true;
};