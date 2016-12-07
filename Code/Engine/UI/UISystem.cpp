#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "ThirdParty/Parsers/XMLParser.hpp"

UISystem* UISystem::instance = nullptr;

//-----------------------------------------------------------------------------------
UISystem::UISystem()
{
}

//-----------------------------------------------------------------------------------
UISystem::~UISystem()
{

}

//-----------------------------------------------------------------------------------
void UISystem::Update(float deltaSeconds)
{
    for (WidgetBase* widget : m_widgets)
    {
        widget->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void UISystem::Render() const
{
    Renderer::instance->BeginOrtho(Vector2::ZERO, Vector2(1600, 900));
    {
        for (WidgetBase* widget : m_widgets)
        {
            widget->Render();
        }
    }
    Renderer::instance->EndOrtho();
}

