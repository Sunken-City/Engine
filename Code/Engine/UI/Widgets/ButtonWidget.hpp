#pragma once
#include "Engine\UI\WidgetBase.hpp"

class LabelWidget;

//-----------------------------------------------------------------------------------
class ButtonWidget : public WidgetBase
{
public:
    ButtonWidget();
    virtual ~ButtonWidget();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void BuildFromXMLNode(XMLNode& node) override;
    virtual AABB2 GetBounds() override;

private:
    std::string m_onClickEventName;
    LabelWidget* m_textLabel;
};