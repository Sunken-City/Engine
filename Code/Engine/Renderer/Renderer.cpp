#include "Engine/Renderer/Renderer.hpp"

Renderer* Renderer::instance = nullptr;

const unsigned char Renderer::plainWhiteTexel[3] = { 255, 255, 255 };

//-----------------------------------------------------------------------------------
Renderer::Renderer(const Vector2Int& windowSize)
    : m_windowSize(windowSize)
{
}

//-----------------------------------------------------------------------------------
Renderer::~Renderer()
{
}