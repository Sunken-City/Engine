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
    WidgetBase* CreateWidget(const std::string& widgetTypeName);
    bool SetWidgetVisibility(const std::string& name, bool setHidden = true);
    static Vector2 ScreenToUIVirtualCoords(const Vector2& cursorPos);

private:
    WidgetBase* CreateWidget(XMLNode& node);
    WidgetBase* FindHighlightedWidget();
    Vector2 GetCursorVirtualPos();
    void AddWidget(WidgetBase* newWidget);

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static UISystem* instance;
    std::vector<WidgetBase*> m_childWidgets;
    WidgetBase* m_highlightedWidget = nullptr;
};