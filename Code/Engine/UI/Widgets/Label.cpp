#include "Engine/UI/Widgets/Label.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

//-----------------------------------------------------------------------------------
Label::Label(XMLNode& node)
    : WidgetBase(node)
{
    const char* textAttribute = node.getAttribute("Text");
    const char* textColorAttribute = node.getAttribute("TextColor");
    const char* textSizeAttribute = node.getAttribute("FontSize");

    ASSERT_OR_DIE(textAttribute != nullptr, "No text was supplied on Label node.");

    std::string text = textAttribute;
    RGBA textColor = textColorAttribute ? RGBA::CreateFromString(textColorAttribute) : RGBA::WHITE;
    float textSize = textSizeAttribute ? std::stof(textSizeAttribute) : 1.0f;

    m_properties.Set("Text", text);
    m_properties.Set<RGBA>("TextColor", textColor);
    m_properties.Set<float>("FontSize", textSize);
}

//-----------------------------------------------------------------------------------
Label::~Label()
{

}

//-----------------------------------------------------------------------------------
void Label::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void Label::Render() const
{
    WidgetBase::Render();

    std::string labelText;
    RGBA textColor;
    float fontSize;

    PropertyGetResult textGet = m_properties.Get<std::string>("Text", labelText);
    PropertyGetResult textColorGet = m_properties.Get<RGBA>("TextColor", textColor);
    PropertyGetResult fontSizeGet = m_properties.Get<float>("FontSize", fontSize);

    Vector2 currentBaseline = Vector2::ONE * 10.0f;
    Renderer::instance->DrawText2D(currentBaseline, labelText, fontSize, textColor, true, BitmapFont::CreateOrGetFont("Runescape"));
}

