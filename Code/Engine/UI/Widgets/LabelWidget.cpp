#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Renderer/AABB2.hpp"

//-----------------------------------------------------------------------------------
LabelWidget::LabelWidget()
    : WidgetBase()
{

}

//-----------------------------------------------------------------------------------
LabelWidget::~LabelWidget()
{

}

//-----------------------------------------------------------------------------------
void LabelWidget::BuildFromXMLNode(XMLNode& node)
{
    std::string name = node.getName();
    if (name == "Label")
    {
        WidgetBase::BuildFromXMLNode(node);
    }

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
    WidgetBase::Render();

    std::string text;
    RGBA textColor;
    float fontSize;

    PropertyGetResult textGet = m_propertiesForAllStates.Get<std::string>("Text", text);
    PropertyGetResult textColorGet = m_propertiesForAllStates.Get<RGBA>("TextColor", textColor);
    PropertyGetResult fontSizeGet = m_propertiesForAllStates.Get<float>("FontSize", fontSize);

    Vector2 currentBaseline = GetParentOffsets() + m_propertiesForAllStates.Get<Vector2>("Offset");
    Renderer::instance->DrawText2D(currentBaseline, text, fontSize, textColor, true, BitmapFont::CreateOrGetFont("Runescape"));
}

