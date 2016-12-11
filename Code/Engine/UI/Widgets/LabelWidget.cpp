#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Renderer/AABB2.hpp"

//-----------------------------------------------------------------------------------
LabelWidget::LabelWidget()
    : WidgetBase()
{
    std::string text = "";
    RGBA textColor = RGBA::WHITE;
    float textSize = 1.0f;

    m_propertiesForAllStates.Set("Text", text);
    m_propertiesForAllStates.Set<RGBA>("TextColor", textColor);
    m_propertiesForAllStates.Set<float>("FontSize", textSize);
}

//-----------------------------------------------------------------------------------
LabelWidget::~LabelWidget()
{

}

//-----------------------------------------------------------------------------------
void LabelWidget::BuildFromXMLNode(XMLNode& node)
{
    WidgetBase::BuildFromXMLNode(node);

    const char* textAttribute = node.getAttribute("Text");
    const char* textColorAttribute = node.getAttribute("TextColor");
    const char* textSizeAttribute = node.getAttribute("FontSize");

    ASSERT_OR_DIE(textAttribute != nullptr, "No text was supplied on Label node.");

    std::string text = textAttribute;
    RGBA textColor = textColorAttribute ? RGBA::CreateFromString(textColorAttribute) : RGBA::WHITE;
    float textSize = textSizeAttribute ? std::stof(textSizeAttribute) : 1.0f;

    m_propertiesForAllStates.Set("Text", text);
    m_propertiesForAllStates.Set<RGBA>("TextColor", textColor);
    m_propertiesForAllStates.Set<float>("FontSize", textSize);
    
    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void LabelWidget::RecalculateBounds()
{
    std::string text;
    float fontSize;

    PropertyGetResult textGet = m_propertiesForAllStates.Get<std::string>("Text", text);
    PropertyGetResult fontSizeGet = m_propertiesForAllStates.Get<float>("FontSize", fontSize);

    m_bounds = BitmapFont::CreateOrGetFont("Runescape")->CalcTextBounds(text, fontSize);
}

//-----------------------------------------------------------------------------------
void LabelWidget::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void LabelWidget::Render() const
{
    std::string text = GetProperty<std::string>("Text");
    RGBA textColor = GetProperty<RGBA>("TextColor");
    float fontSize = GetProperty<float>("FontSize");
    float borderWidth = GetProperty<float>("BorderWidth");
    RGBA bgColor = GetProperty<RGBA>("BackgroundColor");
    RGBA borderColor = GetProperty<RGBA>("BorderColor");
    Vector2 currentBaseline = GetParentOffsets() + GetProperty<Vector2>("Offset");

    if (borderWidth > 0.0f)
    {
        AABB2 borderBounds = m_bounds;
        borderBounds.mins += Vector2(-borderWidth);
        borderBounds.maxs += Vector2(borderWidth);
        Renderer::instance->DrawAABB(borderBounds, borderColor);
    }
    if (bgColor.alpha > 0.0f)
    {
        Renderer::instance->DrawAABB(m_bounds, bgColor);
    }
    Renderer::instance->DrawText2D(currentBaseline, text, fontSize, textColor, true, BitmapFont::CreateOrGetFont("Runescape"));

    WidgetBase::Render();
}

