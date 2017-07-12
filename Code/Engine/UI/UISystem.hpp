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
    void LoadAndParseUIXML(const char* xmlRelativeFilePath = "Data/UI/Widget.xml");
    void ReloadUI(const char* xmlRelativeFilePath = "Data/UI/Widget.xml");
    void DeleteWidget(WidgetBase* widgetToDelete);
    void DeleteAllUI();
    WidgetBase* CreateWidget(const std::string& widgetTypeName);
    WidgetBase* CreateWidget(XMLNode& node);
    WidgetBase* FindWidgetByName(const char* widgetName);
    void AddWidget(WidgetBase* newWidget);
    bool SetWidgetHidden(const std::string& name, bool setHidden = true);
    static Vector2 ScreenToUIVirtualCoords(const Vector2& cursorPos);

private:
    WidgetBase* FindHighlightedWidget();
    Vector2 GetCursorVirtualPos();

public:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    static UISystem* instance;
    std::vector<WidgetBase*> m_childWidgets;
    WidgetBase* m_highlightedWidget = nullptr;
};