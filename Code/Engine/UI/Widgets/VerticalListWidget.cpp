#include "Engine/UI/Widgets/VerticalListWidget.hpp"

//-----------------------------------------------------------------------------------
VerticalListWidget::VerticalListWidget()
{

}

//-----------------------------------------------------------------------------------
VerticalListWidget::~VerticalListWidget()
{

}

//-----------------------------------------------------------------------------------
void VerticalListWidget::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void VerticalListWidget::Render() const
{
    WidgetBase::Render();
    RenderChildren();
}

//-----------------------------------------------------------------------------------
void VerticalListWidget::BuildFromXMLNode(XMLNode& node)
{
    WidgetBase::BuildFromXMLNode(node);
}

//-----------------------------------------------------------------------------------
void VerticalListWidget::RecalculateBounds()
{
    m_bounds = GetSmallestBoundsAroundChildren();
    WidgetBase::RecalculateBounds();
    m_currentBaseline = m_bounds.GetTopLeft();
}

//-----------------------------------------------------------------------------------
void VerticalListWidget::AddChild(WidgetBase* child)
{
    m_children.push_back(child);
    child->m_parent = this;

    child->RecalculateBounds(); //Figure out how tall this component will be first.
    m_currentBaseline.y -= child->m_bounds.GetHeight();
    child->SetProperty<Vector2>("Offset", Vector2(0.0f, m_currentBaseline.y));
    child->RecalculateBounds();
}
