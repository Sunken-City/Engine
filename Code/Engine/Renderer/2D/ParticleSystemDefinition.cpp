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
    m_properties.m_neverChangeTypeIfDifferent = true;
}

//-----------------------------------------------------------------------------------
ParticleEmitterDefinition::ParticleEmitterDefinition(const SpriteResource* spriteResource)
    : m_spriteResource(spriteResource)
{
    m_material = m_spriteResource->m_defaultMaterial; 

    m_properties.m_neverChangeTypeIfDifferent = true;
    m_properties.Set<RGBA>(PROPERTY_INITIAL_COLOR, RGBA::WHITE, false);
    m_properties.Set<bool>(PROPERTY_FADEOUT_ENABLED, false, false);
    m_properties.Set<float>(PROPERTY_PARTICLES_PER_SECOND, 1.0f, false);
    m_properties.Set<Range<unsigned int>>(PROPERTY_INITIAL_NUM_PARTICLES, 0, false);
    m_properties.Set<Range<float>>(PROPERTY_PARTICLE_LIFETIME, 1.0f, false);
    m_properties.Set<Range<float>>(PROPERTY_INITIAL_ROTATION_DEGREES, 0.0f, false);
    m_properties.Set<Range<float>>(PROPERTY_MAX_EMITTER_LIFETIME, 0.0f, false);
    m_properties.Set<Range<float>>(PROPERTY_SPAWN_RADIUS, 0.0f, false);
    m_properties.Set<Range<Vector2>>(PROPERTY_DELTA_SCALE_PER_SECOND, Vector2::ZERO, false);
    m_properties.Set<Range<Vector2>>(PROPERTY_INITIAL_SCALE, Vector2::ONE, false);
    m_properties.Set<Range<Vector2>>(PROPERTY_INITIAL_VELOCITY, Vector2::ZERO, false);
}
