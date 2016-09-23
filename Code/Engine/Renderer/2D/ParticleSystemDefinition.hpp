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
    ParticleEmitterDefinition();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Material* m_material;
    unsigned int m_initialNumParticlesSpawn;
    float m_particlesPerSecond;
    Range<float> m_lifetimePerParticle;
    Range<Vector2> m_initialScalePerParticle;
    Range<Vector2> m_initialVelocity;
    RGBA m_initialTintPerParticle;
    const SpriteResource* m_spriteResource;
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
