#pragma once
#include "Engine/UI/WidgetBase.hpp"

class Label : public WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Label(XMLNode& node);
    ~Label();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
};