#include "Engine/UI/Widgets/Label.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

//-----------------------------------------------------------------------------------
Label::Label(XMLNode& node)
    : WidgetBase(node)
{
    std::string text = node.getAttribute("Text");
    m_properties.Set("Text", text);
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
    m_properties.Get<std::string>("Text", labelText);
    Vector2 currentBaseline = Vector2::ONE * 10.0f;
    Renderer::instance->DrawText2D(currentBaseline, labelText, 1.0f, RGBA::WHITE, true, BitmapFont::CreateOrGetFont("Runescape"));
}

