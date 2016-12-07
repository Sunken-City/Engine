#include "Engine/UI/UISystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

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

}

//-----------------------------------------------------------------------------------
void UISystem::Render() const
{
    Renderer::instance->BeginOrtho(Vector2::ZERO, Vector2(1600, 900));
    {
        Vector2 currentBaseline = Vector2::ONE * 10.0f;
        Renderer::instance->DrawText2D(currentBaseline, std::string("It's meme time"), 1.0f, RGBA::WHITE, true, BitmapFont::CreateOrGetFont("Runescape"));
    }
    Renderer::instance->EndOrtho();
}

