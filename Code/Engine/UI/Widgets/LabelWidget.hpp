#pragma once
#include "Engine/UI/WidgetBase.hpp"

class LabelWidget : public WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    LabelWidget();
    virtual ~LabelWidget();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void BuildFromXMLNode(XMLNode& node) override;
    virtual void RecalculateBounds() override;
};