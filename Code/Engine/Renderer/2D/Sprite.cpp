#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Sprite::Sprite(const std::string& resourceName, int orderingLayer, bool isEnabled) 
    : m_position(Vector2::ZERO)
    , m_scale(Vector2::ONE)
    , m_rotationDegrees(0.0f)
    , m_tintColor(RGBA::WHITE)
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
AABB2 Sprite::GetBounds()
{
    const Vector2 mins((-m_spriteResource->m_pivotPoint.x) * m_scale.x, (-m_spriteResource->m_pivotPoint.y) * m_scale.y);
    const Vector2 maxs((m_spriteResource->m_virtualSize.x - m_spriteResource->m_pivotPoint.x) * m_scale.x, (m_spriteResource->m_virtualSize.y - m_spriteResource->m_pivotPoint.y) * m_scale.y);
    return AABB2(m_position + mins, m_position + maxs);
}

//-----------------------------------------------------------------------------------
AABB2 SpriteResource::GetDefaultBounds() const
{
    const Vector2 mins(-m_pivotPoint.x, -m_pivotPoint.y);
    const Vector2 maxs(m_virtualSize.x - m_pivotPoint.x, m_virtualSize.y - m_pivotPoint.y);
    return AABB2(mins, maxs);
}
