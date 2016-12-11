#include "Engine/UI/WidgetBase.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Core/Events/EventSystem.hpp"

//-----------------------------------------------------------------------------------
WidgetBase::WidgetBase()
    : m_name("Unnamed Widget")
{
    m_propertiesForAllStates.Set<std::string>("Name", m_name);
    m_propertiesForAllStates.Set<Vector2>("Offset", Vector2::ZERO);
    m_propertiesForAllStates.Set<Vector2>("Size", Vector2::ONE);
    m_propertiesForAllStates.Set<RGBA>("BackgroundColor", RGBA::KINDA_GRAY);
    m_propertiesForAllStates.Set<RGBA>("BorderColor", RGBA::KINDA_GRAY);
    m_propertiesForAllStates.Set<float>("Opacity", 1.0f);
    m_propertiesForAllStates.Set<float>("BorderWidth", 0.0f);

    SetProperty("BackgroundColor", RGBA::WHITE, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("BackgroundColor", RGBA::VERY_GRAY, PRESSED_WIDGET_STATE);
    SetProperty("BorderColor", RGBA::WHITE, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("BorderColor", RGBA::VERY_GRAY, PRESSED_WIDGET_STATE);
}

//-----------------------------------------------------------------------------------
WidgetBase::~WidgetBase()
{
    for (WidgetBase* child : m_children)
    {
        delete child;
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::Update(float deltaSeconds)
{
    for (WidgetBase* child : m_children)
    {
        child->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::Render() const
{
    for (WidgetBase* child : m_children)
    {
        child->Render();
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::AddChild(WidgetBase* child)
{
    m_children.push_back(child);
    child->m_parent = this;
}

//-----------------------------------------------------------------------------------
AABB2 WidgetBase::GetSmallestBoundsAroundChildren()
{
    AABB2 smallestBounds = AABB2(Vector2::ZERO, Vector2::ZERO);

    for (WidgetBase* child : m_children)
    {
        AABB2 childBounds = child->GetBounds();
        smallestBounds.mins.x = (childBounds.mins.x < smallestBounds.mins.x) ? childBounds.mins.x : smallestBounds.mins.x;
        smallestBounds.mins.y = (childBounds.mins.y < smallestBounds.mins.y) ? childBounds.mins.y : smallestBounds.mins.y;
        smallestBounds.maxs.x = (childBounds.maxs.x > smallestBounds.maxs.x) ? childBounds.maxs.x : smallestBounds.maxs.x;
        smallestBounds.maxs.y = (childBounds.maxs.y > smallestBounds.maxs.y) ? childBounds.maxs.y : smallestBounds.maxs.y;
    }

    return smallestBounds;
}

//-----------------------------------------------------------------------------------
void WidgetBase::BuildFromXMLNode(XMLNode& node)
{
    const char* nameAttribute = node.getAttribute("Name");
    const char* horizontalOffset = node.getAttribute("HorizontalOffset");
    const char* verticalOffset = node.getAttribute("VerticalOffset");
    const char* backgroundColorAttribute = node.getAttribute("BackgroundColor");
    const char* borderColorAttribute = node.getAttribute("BorderColor");
    const char* borderWidthAttribute = node.getAttribute("BorderWidth");
    const char* onClickAttribute = node.getAttribute("OnClick");
    const char* h_backgroundColorAttribute = node.getAttribute("H_BackgroundColor");
    const char* h_borderColorAttribute = node.getAttribute("H_BorderColor");
    const char* p_backgroundColorAttribute = node.getAttribute("P_BackgroundColor");
    const char* p_borderColorAttribute = node.getAttribute("P_BorderColor");

    Vector2 offset = m_propertiesForAllStates.Get<Vector2>("Offset");
    RGBA bgColor = m_propertiesForAllStates.Get<RGBA>("BackgroundColor");
    RGBA edgeColor = m_propertiesForAllStates.Get<RGBA>("BorderColor");
    float borderWidth = m_propertiesForAllStates.Get<float>("BorderWidth");

    if (horizontalOffset)
    {
        offset.x = std::stof(horizontalOffset);
    }
    if (verticalOffset)
    {
        offset.y = std::stof(verticalOffset);
    }
    if (backgroundColorAttribute)
    {
        bgColor = RGBA::CreateFromString(backgroundColorAttribute);
    }
    if (borderColorAttribute)
    {
        edgeColor = RGBA::CreateFromString(borderColorAttribute);
    }
    if (borderWidthAttribute)
    {
        borderWidth = std::stof(borderWidthAttribute);
    }
    if (onClickAttribute)
    {
        std::string eventName = std::string(onClickAttribute);
        m_propertiesForAllStates.Set("OnClick", eventName);
    }
    if (nameAttribute)
    {
        std::string name = std::string(nameAttribute);
        m_propertiesForAllStates.Set("Name", name);
        m_name = name;
    }

    if (h_backgroundColorAttribute)
    {
        SetProperty("BackgroundColor", RGBA::CreateFromString(h_backgroundColorAttribute), HIGHLIGHTED_WIDGET_STATE);
    }
    else
    {
        RGBA highlightedColor = bgColor + RGBA(0x22222200);
        SetProperty("BackgroundColor", highlightedColor, HIGHLIGHTED_WIDGET_STATE);
    }

    if (h_borderColorAttribute)
    {
        SetProperty("BorderColor", RGBA::CreateFromString(h_borderColorAttribute), HIGHLIGHTED_WIDGET_STATE);
    }
    else
    {
        RGBA highlightedColor = edgeColor + RGBA(0x22222200);
        SetProperty("BorderColor", highlightedColor, HIGHLIGHTED_WIDGET_STATE);
    }

    if (p_backgroundColorAttribute)
    {
        SetProperty("BackgroundColor", RGBA::CreateFromString(p_backgroundColorAttribute), PRESSED_WIDGET_STATE);
    }
    else
    {
        RGBA pressedColor = bgColor - RGBA(0x22222200);
        SetProperty("BackgroundColor", pressedColor, PRESSED_WIDGET_STATE);
    }

    if (p_borderColorAttribute)
    {
        SetProperty("BorderColor", RGBA::CreateFromString(p_borderColorAttribute), PRESSED_WIDGET_STATE);
    }
    else
    {
        RGBA pressedColor = edgeColor - RGBA(0x22222200);
        SetProperty("BorderColor", pressedColor, PRESSED_WIDGET_STATE);
    }

    m_propertiesForAllStates.Set<Vector2>("Offset", offset);
    m_propertiesForAllStates.Set<RGBA>("BackgroundColor", bgColor);
    m_propertiesForAllStates.Set<RGBA>("BorderColor", edgeColor);
    m_propertiesForAllStates.Set<Vector2>("Size", Vector2::ONE);
    m_propertiesForAllStates.Set<float>("Opacity", 1.0f);
    m_propertiesForAllStates.Set<float>("BorderWidth", borderWidth);
}

//-----------------------------------------------------------------------------------
void WidgetBase::OnClick()
{
    std::string clickEvent;
    PropertyGetResult state = m_propertiesForAllStates.Get("OnClick", clickEvent);
    if (state == PGR_SUCCESS)
    {
        EventSystem::FireEvent(clickEvent);
    }
    UnsetPressed();
}

//-----------------------------------------------------------------------------------
Vector2 WidgetBase::GetParentOffsets() const
{
    Vector2 parentOffsets = Vector2::ZERO;
    WidgetBase* parent = m_parent;
    while (parent)
    {
        parentOffsets += parent->m_propertiesForAllStates.Get<Vector2>("Offset");
        parent = parent->m_parent;
    }
    return parentOffsets;
}

//-----------------------------------------------------------------------------------
Matrix4x4 WidgetBase::GetModelMatrix() const
{
    Matrix4x4 model = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeTranslation(&model, Vector3(m_propertiesForAllStates.Get<Vector2>("Offset"), 0.0f));
    return model;
}

//-----------------------------------------------------------------------------------
WidgetBase* WidgetBase::GetWidgetPointIsInside(const Vector2& point)
{
    for (WidgetBase* child : m_children) //Ask in the appropriate drawing order
    {
        WidgetBase* widget = child->GetWidgetPointIsInside(point);
        if (widget)
        {
            return widget;
        }
    }
    return m_bounds.IsPointOnOrInside(point) ? this : nullptr;
}