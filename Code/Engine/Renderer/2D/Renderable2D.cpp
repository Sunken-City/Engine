#include "Engine/Renderer/2D/Renderable2D.hpp"
#include "SpriteGameRenderer.hpp"

//-----------------------------------------------------------------------------------
Renderable2D::Renderable2D()
{

}

//-----------------------------------------------------------------------------------
Renderable2D::~Renderable2D()
{
    Disable();
}

//-----------------------------------------------------------------------------------
void Renderable2D::ChangeLayer(int layer)
{
    //Disable if already connected.
    Disable();
    m_orderingLayer = layer;
    Enable();
}

//-----------------------------------------------------------------------------------
void Renderable2D::Enable()
{
    if (!m_isEnabled)
    {
        //SpriteGameRenderer::instance->RegisterSprite(this);
        m_isEnabled = true;
    }
}

//-----------------------------------------------------------------------------------
void Renderable2D::Disable()
{
    if (m_isEnabled)
    {
        //SpriteGameRenderer::instance->UnregisterSprite(this);
        m_isEnabled = false;
    }
}