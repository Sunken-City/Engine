#pragma once
#include <vector>

class WidgetBase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    WidgetBase();
    virtual ~WidgetBase();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void AddChild(WidgetBase* child);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    WidgetBase* m_parent = nullptr;
    std::vector<WidgetBase*> m_children;
};