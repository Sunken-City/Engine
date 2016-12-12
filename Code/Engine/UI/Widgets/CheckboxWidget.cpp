#include "Engine\UI\Widgets\CheckboxWidget.hpp"
#include "..\..\Renderer\Renderer.hpp"
#include "..\..\Fonts\BitmapFont.hpp"

//-----------------------------------------------------------------------------------
CheckboxWidget::CheckboxWidget()
    : ButtonWidget()
{
    SetProperty("BackgroundColor", RGBA::WHITE, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("BackgroundColor", RGBA::GRAY, PRESSED_WIDGET_STATE);
    SetProperty("BorderColor", RGBA::WHITE, HIGHLIGHTED_WIDGET_STATE);
    SetProperty("BorderColor", RGBA::VERY_GRAY, PRESSED_WIDGET_STATE);
    SetProperty<std::string>("DefaultState", "Unchecked");
}

//-----------------------------------------------------------------------------------
CheckboxWidget::~CheckboxWidget()
{

}

//-----------------------------------------------------------------------------------
void CheckboxWidget::Update(float deltaSeconds)
{
    LabelWidget::Update(deltaSeconds);
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void CheckboxWidget::Render() const
{
    if (IsHidden())
    {
        return;
    }
    float borderWidth = GetProperty<float>("BorderWidth");
    RGBA bgColor = GetProperty<RGBA>("BackgroundColor");
    RGBA borderColor = GetProperty<RGBA>("BorderColor");
    float opacity = GetProperty<float>("Opacity");
    std::string text = GetProperty<std::string>("Text");
    float textOpacity = GetProperty<float>("TextOpacity");
    float fontSize = GetProperty<float>("TextSize");
    RGBA textColor = GetProperty<RGBA>("TextColor");
    Vector2 currentBaseline = GetTotalOffset() + GetProperty<Vector2>("TextOffset");

    opacity *= GetParentOpacities();
    bgColor.alpha = (uchar)((((float)bgColor.alpha / 255.0f) * opacity) * 255.0f);
    borderColor.alpha = (uchar)((((float)borderColor.alpha / 255.0f) * opacity) * 255.0f);
    textColor.alpha = (uchar)((((float)textColor.alpha / 255.0f) * (opacity * textOpacity)) * 255.0f);

    if (borderWidth > 0.0f)
    {
        AABB2 borderBounds = m_checkboxBounds;
        borderBounds.mins += Vector2(-borderWidth);
        borderBounds.maxs += Vector2(borderWidth);
        Renderer::instance->DrawAABB(borderBounds, borderColor);
    }
    if (bgColor.alpha > 0.0f)
    {
        Renderer::instance->DrawAABB(m_checkboxBounds, bgColor);
    }

    if (m_isChecked)
    {
        Renderer::instance->DrawText2D(m_checkboxBounds.mins, "X", fontSize, textColor, true, BitmapFont::CreateOrGetFont("Runescape"));
    }

    Renderer::instance->DrawText2D(currentBaseline, text, fontSize, textColor, true, BitmapFont::CreateOrGetFont("Runescape"));
    RenderChildren();
}

//-----------------------------------------------------------------------------------
void CheckboxWidget::BuildFromXMLNode(XMLNode& node)
{
    ButtonWidget::BuildFromXMLNode(node);

    const char* defaultStateAttribute = node.getAttribute("DefaultState");

    if (defaultStateAttribute)
    {
        if (strcmp(defaultStateAttribute, "Checked") == 0)
        {
            m_isChecked = true;
        }
        else if (strcmp(defaultStateAttribute, "Unchecked") == 0)
        {
            m_isChecked = false;
        }
        else
        {
            ERROR_RECOVERABLE("Had a value for checkbox's DefaultState, but it wasn't valid.");
        }
    }

    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void CheckboxWidget::RecalculateBounds()
{
    std::string text;
    float fontSize;

    PropertyGetResult textGet = m_propertiesForAllStates.Get<std::string>("Text", text);
    PropertyGetResult fontSizeGet = m_propertiesForAllStates.Get<float>("TextSize", fontSize);
    AABB2 checkboxSize = BitmapFont::CreateOrGetFont("Runescape")->CalcTextBounds("X", fontSize);
    float checkboxWidth = checkboxSize.GetWidth();
    float checkboxHeight = checkboxSize.GetHeight();
    float borderWidth = GetProperty<float>("BorderWidth");

    m_bounds = BitmapFont::CreateOrGetFont("Runescape")->CalcTextBounds(text, fontSize);
    m_bounds += GetTotalOffset();

    m_checkboxBounds = m_bounds;
    m_checkboxBounds.mins.x -= checkboxWidth;
    m_checkboxBounds.maxs = m_bounds.mins + Vector2(0.0f, checkboxHeight);
    m_bounds.mins = m_checkboxBounds.mins;
    m_bounds.mins += Vector2(-borderWidth);
    m_bounds.maxs += Vector2(borderWidth);

    m_bounds.mins -= m_propertiesForAllStates.Get<Vector2>("Padding");
    m_bounds.maxs += m_propertiesForAllStates.Get<Vector2>("Padding");
}

//-----------------------------------------------------------------------------------
void CheckboxWidget::OnClick()
{
    WidgetBase::OnClick();
    m_isChecked = !m_isChecked;
}
