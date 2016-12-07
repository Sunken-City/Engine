#include "Engine/UI/WidgetBase.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

//-----------------------------------------------------------------------------------
WidgetBase::WidgetBase()
{
    m_properties.Set<Vector2>("Offset", Vector2::ZERO);
    m_properties.Set<Vector2>("Size", Vector2::ONE);
    m_properties.Set<RGBA>("BackgroundColor", RGBA::WHITE);
    m_properties.Set<RGBA>("EdgeColor", RGBA::WHITE);
    m_properties.Set<float>("Opacity", 1.0f);
}

//-----------------------------------------------------------------------------------
WidgetBase::WidgetBase(XMLNode& node)
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
}

//-----------------------------------------------------------------------------------
void WidgetBase::AddChild(WidgetBase* child)
{
    m_children.push_back(child);
}

