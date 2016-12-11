#include "Engine/UI/Widgets/WindowWidget.hpp"

//-----------------------------------------------------------------------------------
WindowWidget::WindowWidget()
{

}

//-----------------------------------------------------------------------------------
WindowWidget::~WindowWidget()
{

}

//-----------------------------------------------------------------------------------
void WindowWidget::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void WindowWidget::Render() const
{
    WidgetBase::Render();
    RenderChildren();
}

//-----------------------------------------------------------------------------------
void WindowWidget::BuildFromXMLNode(XMLNode& node)
{

}

//-----------------------------------------------------------------------------------
void WindowWidget::RecalculateBounds()
{
    m_bounds = GetSmallestBoundsAroundChildren();
}
