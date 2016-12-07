#pragma once
#include "Engine/UI/WidgetBase.hpp"
#include <vector>

class UISystem
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    UISystem();
    ~UISystem();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render() const;
    //WidgetBase* CreateWidget

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static UISystem* instance;
    std::vector<WidgetBase*> m_widgets;
};