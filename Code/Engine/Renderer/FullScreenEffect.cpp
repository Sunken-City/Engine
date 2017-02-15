#include "Engine/Renderer/FullScreenEffect.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"

//-----------------------------------------------------------------------------------
FullScreenEffect::FullScreenEffect(Material* material) 
    : m_material(material)
    , m_visibilityFilter((unsigned int)SpriteGameRenderer::PlayerVisibility::ALL)
{
    
}
