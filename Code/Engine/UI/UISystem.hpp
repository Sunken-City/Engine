#pragma once
#include "Engine/UI/WidgetBase.hpp"
#include "Engine/Input/XMLUtils.hpp"
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
    void LoadAndParseUIXML();
    void ReloadUI();
    void DeleteAllUI();
    WidgetBase* CreateWidget(const std::string& name);

private:
    WidgetBase* CreateWidget(XMLNode& node);

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static UISystem* instance;
    std::vector<WidgetBase*> m_childWidgets;
};