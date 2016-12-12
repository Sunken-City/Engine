#pragma once
#include "Engine\UI\Widgets\ButtonWidget.hpp"
#include "Engine\Renderer\AABB2.hpp"

//-----------------------------------------------------------------------------------
class CheckboxWidget : public ButtonWidget
{
public:
    CheckboxWidget();
    virtual ~CheckboxWidget();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds) override;
    virtual void Render() const override;
    virtual void BuildFromXMLNode(XMLNode& node) override;
    virtual void RecalculateBounds() override;
    virtual void OnClick() override;

    ////MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    bool m_isChecked = false;
private:
    AABB2 m_checkboxBounds;
};
