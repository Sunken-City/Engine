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
    WidgetBase::BuildFromXMLNode(node);
    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void WindowWidget::RecalculateBounds()
{
    float borderWidth = GetProperty<float>("BorderWidth");
    m_bounds = GetSmallestBoundsAroundChildren();
    ApplyOffsetProperty();
    ApplySizeProperty();
    m_borderlessBounds = m_bounds;

    m_bounds.mins += Vector2(-borderWidth);
    m_bounds.maxs += Vector2(borderWidth);
    m_borderedBounds = m_bounds;

    ApplyPaddingProperty();
}
