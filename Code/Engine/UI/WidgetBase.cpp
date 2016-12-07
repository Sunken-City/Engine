#include "Engine/UI/WidgetBase.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

//-----------------------------------------------------------------------------------
WidgetBase::WidgetBase()
{

}

//-----------------------------------------------------------------------------------
WidgetBase::~WidgetBase()
{

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

    Vector2 currentBaseline = Vector2::ONE * 10.0f;
    Renderer::instance->DrawText2D(currentBaseline, std::string("It's meme time"), 1.0f, RGBA::WHITE, true, BitmapFont::CreateOrGetFont("Runescape"));

}

//-----------------------------------------------------------------------------------
void WidgetBase::AddChild(WidgetBase* child)
{
    m_children.push_back(child);
}

