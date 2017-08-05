#pragma once
#include <vector>
#include "Engine/Input/XMLUtils.hpp"
#include "Engine/Core/Events/NamedProperties.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "../Math/Transform2D.hpp"
#include "Dimensions.hpp"

class Matrix4x4;
class Vector2;
class Texture;
class Material;

//-----------------------------------------------------------------------------------
enum WidgetState
{
    ACTIVE_WIDGET_STATE,
    HIGHLIGHTED_WIDGET_STATE,
    PRESSED_WIDGET_STATE,
    DISABLED_WIDGET_STATE,
    HIDDEN_WIDGET_STATE,
    NUM_WIDGET_STATES,
    DEFAULT_STATE = NUM_WIDGET_STATES
};

//-----------------------------------------------------------------------------------
enum DockPosition
{
    NOT_DOCKED,
    BOTTOM_DOCKED,
    TOP_DOCKED,
    LEFT_DOCKED,
    RIGHT_DOCKED,
    FILL_DOCKED,
    NUM_DOCKING_POSITIONS
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
    void UpdateChildren(float deltaSeconds);
    virtual void Render() const;
    void RenderChildren() const;
    virtual void AddChild(WidgetBase* child);
    virtual AABB2 GetBounds() { return m_bounds; };
    virtual AABB2 GetSmallestBoundsAroundChildren();
    virtual void RecalculateBounds() = 0;
    virtual void ApplyBorderProperty();
    virtual void ApplySizeProperty();
    virtual void ApplyPaddingProperty();
    virtual void ApplyMarginProperty();
    virtual void ApplyDockingProperty();
    virtual inline void ApplyOffsetProperty() { m_position = GetTotalOffset(); };

    virtual void BuildFromXMLNode(XMLNode& node);
    virtual void OnClick();
    virtual WidgetBase* GetWidgetPointIsInside(const Vector2& point); //Is this inside you or any of your children?
    virtual WidgetBase* FindWidgetByName(const char* widgetName);
    virtual void SetHighlighted() { SetState(HIGHLIGHTED_WIDGET_STATE); };
    virtual void UnsetHighlighted() { RevertToPreviousState(); };
    virtual void SetPressed() { SetState(PRESSED_WIDGET_STATE, false); };
    virtual void UnsetPressed() { RevertToPreviousState(); };
    inline bool IsHighlighted() const { return m_currentState == HIGHLIGHTED_WIDGET_STATE; };
    inline bool IsHidden() const { return m_currentState == HIDDEN_WIDGET_STATE; };
    void SetHidden();
    void SetVisible();
    inline Vector2 GetTotalOffset() const { return GetParentOffsets() + GetProperty<Vector2>("Offset"); };
    bool IsClickable();
    bool SetWidgetVisibility(const std::string& name, bool setHidden = true);
    Vector2 GetParentOffsets() const;
    float GetParentOpacities() const;
    Matrix4x4 GetModelMatrix() const;
    AABB2 GetWorldBounds() const { return m_bounds + m_position; };

private:
    void SetState(WidgetState newState, bool updatePreviousState = true);
    void RevertToPreviousState();

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    mutable NamedProperties m_propertiesForAllStates;
    mutable NamedProperties m_propertiesForState[NUM_WIDGET_STATES];
    std::vector<WidgetBase*> m_children;
    std::string m_name;
    std::string m_textureName;
    Dimensions2D m_dimensions;
    Vector2 m_position = Vector2::ZERO;
    AABB2 m_bounds;
    AABB2 m_borderedBounds;
    AABB2 m_borderlessBounds;
    Texture* m_texture = nullptr;
    Material* m_material = nullptr;
    WidgetBase* m_parent = nullptr;
    WidgetState m_previousState = ACTIVE_WIDGET_STATE;
    WidgetState m_currentState = ACTIVE_WIDGET_STATE;
    DockPosition m_dockType = NOT_DOCKED;
    float m_timeInState = 0.0f;
    mutable float m_currentScale = 1.0f;
    const bool m_isInteractive = true;
};