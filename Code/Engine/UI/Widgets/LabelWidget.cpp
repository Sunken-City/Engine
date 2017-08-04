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
    float textOpacity = 1.0f;

    SetProperty<std::string>("Text", text);
    SetProperty<RGBA>("TextColor", textColor);
    SetProperty<float>("TextSize", textSize);
    SetProperty<float>("TextOpacity", textOpacity);
    SetProperty<Vector2>("TextOffset", Vector2::ZERO);
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
    const char* textSizeAttribute = node.getAttribute("TextSize");
    const char* textOpacityAttribute = node.getAttribute("TextOpacity");
    const char* textOffsetAttribute = node.getAttribute("TextOffset");

    if (textAttribute == nullptr)
    {
        textAttribute = "";
    }

    std::string text = textAttribute;
    RGBA textColor = textColorAttribute ? RGBA::CreateFromString(textColorAttribute) : RGBA::WHITE;
    float textSize = textSizeAttribute ? std::stof(textSizeAttribute) : 1.0f;
    float textOpacity = textOpacityAttribute ? std::stof(textOpacityAttribute) : 1.0f;
    Vector2 textOffset = textOffsetAttribute ? Vector2::CreateFromString(textOffsetAttribute) : Vector2::ZERO;

    m_propertiesForAllStates.Set("Text", text);
    m_propertiesForAllStates.Set<RGBA>("TextColor", textColor);
    m_propertiesForAllStates.Set<float>("TextSize", textSize);
    m_propertiesForAllStates.Set<float>("TextOpacity", textOpacity);
    SetProperty<Vector2>("TextOffset", textOffset);
}

//-----------------------------------------------------------------------------------
void LabelWidget::RecalculateBounds()
{
    std::string text;
    float fontSize;
    PropertyGetResult textGet = m_propertiesForAllStates.Get<std::string>("Text", text);
    PropertyGetResult fontSizeGet = m_propertiesForAllStates.Get<float>("TextSize", fontSize);

    m_bounds = BitmapFont::CreateOrGetFont("Runescape")->CalcTextBounds(text, fontSize);
    Vector2 minSize = Vector2(m_bounds.GetWidth(), m_bounds.GetHeight());
    Vector2 currentMinSize = m_dimensions.GetMinSize();
    
    //If the user has defined a Size already, we want the bigger of the two sizes so the text is guaranteed to be wrapped.
    float x = std::max(minSize.x, currentMinSize.x);
    float y = std::max(minSize.y, currentMinSize.y);
    Vector2 maxOfMinSizes = Vector2(x, y);
    m_dimensions.SetMinSize(maxOfMinSizes);

    WidgetBase::RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void LabelWidget::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void LabelWidget::Render() const
{
    WidgetBase::Render();

    std::string text = GetProperty<std::string>("Text");
    float textOpacity = GetProperty<float>("TextOpacity");
    float fontSize = GetProperty<float>("TextSize");
    float opacity = GetProperty<float>("Opacity");
    RGBA textColor = GetProperty<RGBA>("TextColor");
    Vector2 currentBaseline = GetTotalOffset() + GetProperty<Vector2>("TextOffset");

    opacity *= GetParentOpacities();
    textColor.alpha = (uchar)((((float)textColor.alpha / 255.0f) * (opacity * textOpacity)) * 255.0f);

    Renderer::instance->DrawText2D(currentBaseline, text, fontSize, textColor, true, BitmapFont::CreateOrGetFont("Runescape"));
    RenderChildren();
}

