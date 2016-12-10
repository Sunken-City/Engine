#pragma once
#include "Engine\UI\Widgets\LabelWidget.hpp"

//-----------------------------------------------------------------------------------
class ButtonWidget : public LabelWidget
{
public:
    ButtonWidget();
    virtual ~ButtonWidget();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void BuildFromXMLNode(XMLNode& node) override;
    virtual void RecalculateBounds() override;
};