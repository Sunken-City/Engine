#include "Engine/UI/Widgets/ButtonWidget.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"

//-----------------------------------------------------------------------------------
ButtonWidget::ButtonWidget()
    : WidgetBase()
{
    m_textLabel = new LabelWidget();
    m_children.push_back(m_textLabel);
}

//-----------------------------------------------------------------------------------
ButtonWidget::~ButtonWidget()
{
    delete m_textLabel;
}

//-----------------------------------------------------------------------------------
void ButtonWidget::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void ButtonWidget::Render() const
{
    WidgetBase::Render();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::BuildFromXMLNode(XMLNode& node)
{
    m_textLabel->BuildFromXMLNode(node);
}

//-----------------------------------------------------------------------------------
AABB2 ButtonWidget::GetBounds()
{
    return AABB2();
}
