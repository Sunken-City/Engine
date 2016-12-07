#pragma once
#include <vector>
#include "Engine/Input/XMLUtils.hpp"
#include "Engine/Core/Events/NamedProperties.hpp"

class WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    WidgetBase();
    WidgetBase(XMLNode& node);
    virtual ~WidgetBase();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void AddChild(WidgetBase* child);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    WidgetBase* m_parent = nullptr;
    std::vector<WidgetBase*> m_children;
    mutable NamedProperties m_properties;
};