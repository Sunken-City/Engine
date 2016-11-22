#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include <vector>

class Material;
class SpriteResource;

//-----------------------------------------------------------------------------------
enum ParticleSystemType
{
    ONE_SHOT,
    LOOPING,
    NUM_SYSTEMS
};

//-----------------------------------------------------------------------------------
class ParticleEmitterDefinition
{
public:
    ParticleEmitterDefinition(const SpriteResource* spriteResource);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Material* m_material;
    unsigned int m_initialNumParticlesSpawn;
    float m_particlesPerSecond;
    Range<float> m_lifetimePerParticle;
    Range<Vector2> m_initialScalePerParticle;
    Range<Vector2> m_initialVelocity;
    RGBA m_initialTintPerParticle;
    bool m_fadeoutEnabled;
    const SpriteResource* m_spriteResource;
    Range<float> m_maxLifetime = 0.0f;
    Range<float> m_spawnRadius = 0.0f; 
    Range<float> m_initialRotationDegrees = 0.0f;
    Vector2 m_scaleRateOfChangePerSecond = Vector2::ONE;
};

//-----------------------------------------------------------------------------------
class ParticleSystemDefinition
{
public:
    ParticleSystemDefinition(ParticleSystemType type) : m_type(type) {};
    ~ParticleSystemDefinition();
    void AddEmitter(ParticleEmitterDefinition* emitterDefinition);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<ParticleEmitterDefinition*> m_emitterDefinitions;
    ParticleSystemType m_type;
    ParticleSystemDefinition* next;
    ParticleSystemDefinition* prev;
};
