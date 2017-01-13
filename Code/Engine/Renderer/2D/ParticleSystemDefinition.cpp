#include "Engine/Renderer/2D/ParticleSystemDefinition.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
ParticleSystemDefinition::~ParticleSystemDefinition()
{
    for (ParticleEmitterDefinition* emitter : m_emitterDefinitions)
    {
        delete emitter;
    }
}

//-----------------------------------------------------------------------------------
void ParticleSystemDefinition::AddEmitter(ParticleEmitterDefinition* emitter)
{
    m_emitterDefinitions.push_back(emitter);
}

//-----------------------------------------------------------------------------------
ParticleEmitterDefinition::ParticleEmitterDefinition(const SpriteResource* spriteResource)
    : m_initialScalePerParticle(Vector2::ONE)
    , m_lifetimePerParticle(1.0f)
    , m_initialTintPerParticle(RGBA::WHITE)
    , m_initialVelocity(Vector2::ZERO)
    , m_initialNumParticlesSpawn(0)
    , m_particlesPerSecond(1.0f)
    , m_spriteResource(spriteResource)
{
    m_material = m_spriteResource->m_defaultMaterial; 
    //m_properties.Set<bool>("Fadeout Enabled", false);
}
