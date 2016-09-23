#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Sprite::Sprite(const std::string& resourceName, int orderingLayer, bool isEnabled) 
    : m_position(Vector2::ZERO)
    , m_scale(Vector2::ONE)
    , m_rotationDegrees(0.0f)
    , m_tintColor(RGBA::WHITE)
    , m_isEnabled(false)
    , m_material(nullptr)
    , prev(nullptr)
    , next(nullptr)
{
    this->m_spriteResource = ResourceDatabase::instance->GetSpriteResource(resourceName);
    this->m_material = m_spriteResource->m_defaultMaterial;
    if (isEnabled)
    {
        m_orderingLayer = orderingLayer;
        Enable();
    }
}

//-----------------------------------------------------------------------------------
Sprite::~Sprite()
{
    Disable();
}

//-----------------------------------------------------------------------------------
void Sprite::ChangeLayer(int layer)
{
    //Disable if already connected.
    Disable();
    m_orderingLayer = layer;
    Enable();
}

//-----------------------------------------------------------------------------------
void Sprite::Enable()
{
    if (!m_isEnabled) 
    {
        SpriteGameRenderer::instance->RegisterSprite(this);
        m_isEnabled = true;
    }
}

//-----------------------------------------------------------------------------------
void Sprite::Disable()
{
    if (m_isEnabled) 
    {
        SpriteGameRenderer::instance->UnregisterSprite(this);
        m_isEnabled = false;
    }
}

//-----------------------------------------------------------------------------------
AABB2 Sprite::GetBounds()
{
    const Vector2 mins((-m_spriteResource->m_pivotPoint.x) * m_scale.x, (-m_spriteResource->m_pivotPoint.y) * m_scale.y);
    const Vector2 maxs((m_spriteResource->m_virtualSize.x - m_spriteResource->m_pivotPoint.x) * m_scale.x, (m_spriteResource->m_virtualSize.y - m_spriteResource->m_pivotPoint.y) * m_scale.y);
    return AABB2(m_position + mins, m_position + maxs);
}
