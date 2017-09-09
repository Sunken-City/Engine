#pragma once
#include "Engine/UI/Widgets/WindowWidget.hpp"

class VerticalListWidget : public WindowWidget
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    VerticalListWidget();
    virtual ~VerticalListWidget();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds) override;
    virtual void Render() const override;
    virtual void BuildFromXMLNode(XMLNode& node) override;
    virtual void RecalculateBounds() override;
    virtual void AddChild(WidgetBase* child) override;

    ////MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Vector2 m_currentBaseline = Vector2::ZERO;
}; 
