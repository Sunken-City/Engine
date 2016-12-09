#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Widgets/LabelWidget.hpp"
#include "Widgets/ButtonWidget.hpp"
#include "../Input/InputSystem.hpp"

UISystem* UISystem::instance = nullptr;

//-----------------------------------------------------------------------------------
UISystem::UISystem()
{
    LoadAndParseUIXML();
}

//-----------------------------------------------------------------------------------
UISystem::~UISystem()
{
    DeleteAllUI();
}

//-----------------------------------------------------------------------------------
void UISystem::Update(float deltaSeconds)
{
    if (InputSystem::instance->WasKeyJustPressed('U'))
    {
        ReloadUI();
    }
    for (WidgetBase* widget : m_childWidgets)
    {
        widget->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void UISystem::Render() const
{
    Renderer::instance->ClearDepth();
    Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::OFF;
    Renderer::instance->BeginOrtho(Vector2::ZERO, Vector2(1600, 900));
    {
        for (WidgetBase* widget : m_childWidgets)
        {
            widget->Render();
        }
    }
    Renderer::instance->EndOrtho();
    Renderer::instance->m_defaultMaterial->m_renderState.depthTestingMode = RenderState::DepthTestingMode::ON;
}

//-----------------------------------------------------------------------------------
void UISystem::LoadAndParseUIXML()
{
    XMLNode root = XMLUtils::OpenXMLDocument("Data/UI/Widget.xml");
    std::vector<XMLNode> children = XMLUtils::GetChildren(root);
    for (XMLNode& node : children)
    {
        if (!node.isEmpty())
        {
            m_childWidgets.push_back(CreateWidget(node));
        }
    }
}

//-----------------------------------------------------------------------------------
void UISystem::ReloadUI()
{
    DeleteAllUI();
    LoadAndParseUIXML();
}

//-----------------------------------------------------------------------------------
void UISystem::DeleteAllUI()
{
    for (WidgetBase* child : m_childWidgets)
    {
        delete child;
    }
    m_childWidgets.clear();
}

//-----------------------------------------------------------------------------------
WidgetBase* UISystem::CreateWidget(XMLNode& node)
{
    std::string nodeName = node.getName();
    WidgetBase* widget = CreateWidget(nodeName);
    widget->BuildFromXMLNode(node);
    return widget;
}

//-----------------------------------------------------------------------------------
WidgetBase* UISystem::CreateWidget(const std::string& name)
{
    if (name == "Label")
    {
        return static_cast<WidgetBase*>(new LabelWidget());
    }
    else if (name == "Button")
    {
        return static_cast<WidgetBase*>(new ButtonWidget());
    }
    return nullptr;
}
