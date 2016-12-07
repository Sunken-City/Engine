#pragma once
#include "Engine/UI/WidgetBase.hpp"

class Label : public WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Label();
    Label(XMLNode& node);
    virtual ~Label();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
};