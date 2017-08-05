#include "Engine/UI/Widgets/ButtonWidget.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/UI/UISystem.hpp"

//-----------------------------------------------------------------------------------
ButtonWidget::ButtonWidget()
    : LabelWidget()
{
    SetProperty("BackgroundColor", RGBA::WHITE, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("BackgroundColor", RGBA::GRAY, PRESSED_WIDGET_STATE);
    SetProperty("BorderColor", RGBA::WHITE, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("BorderColor", RGBA::DARK_GRAY, PRESSED_WIDGET_STATE);
    SetProperty("Scale", 1.1f, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("Scale", 0.9f, PRESSED_WIDGET_STATE);
}

//-----------------------------------------------------------------------------------
ButtonWidget::~ButtonWidget()
{

}

//-----------------------------------------------------------------------------------
void ButtonWidget::Update(float deltaSeconds)
{
    static Vector2 lastSize = GetProperty<Vector2>("Size");
    Vector2 currentSize = GetProperty<Vector2>("Size");
    if (lastSize != currentSize)
    {
        lastSize = currentSize;
        RecalculateBounds();
    }
    LabelWidget::Update(deltaSeconds);
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void ButtonWidget::Render() const
{
    LabelWidget::Render();
    RenderChildren();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::BuildFromXMLNode(XMLNode& node)
{
    LabelWidget::BuildFromXMLNode(node);

    const char* h_backgroundColorAttribute = node.getAttribute("H_BackgroundColor");
    const char* h_borderColorAttribute = node.getAttribute("H_BorderColor");
    const char* p_backgroundColorAttribute = node.getAttribute("P_BackgroundColor");
    const char* p_borderColorAttribute = node.getAttribute("P_BorderColor");
    const char* hoverScaleAttribute = node.getAttribute("HoverScale");
    const char* pressedScaleAttribute = node.getAttribute("PressedScale");

    RGBA bgColor = m_propertiesForAllStates.Get<RGBA>("BackgroundColor");
    RGBA edgeColor = m_propertiesForAllStates.Get<RGBA>("BorderColor");
    float hoverScale = 1.1f;
    float pressedScale = 0.9f;

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

    if (hoverScaleAttribute)
    {
        hoverScale = GetFloatFromString(hoverScaleAttribute);
    }
    if (pressedScaleAttribute)
    {
        pressedScale = GetFloatFromString(pressedScaleAttribute);
    }
    SetProperty("Scale", hoverScale, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("Scale", pressedScale, PRESSED_WIDGET_STATE);

    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::RecalculateBounds()
{
    LabelWidget::RecalculateBounds();
}