#pragma once
#include "Engine/UI/WidgetBase.hpp"

class WindowWidget : public WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    WindowWidget();
    virtual ~WindowWidget();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds) override;
    virtual void Render() const override;
    virtual void BuildFromXMLNode(XMLNode& node) override;
    virtual void RecalculateBounds() override;
};