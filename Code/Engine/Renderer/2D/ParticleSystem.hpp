#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include <vector>

class ParticleEmitterDefinition;
class ParticleSystemDefinition;

//-----------------------------------------------------------------------------------
struct Particle
{
    Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, const Vector2& initalVelocity = Vector2::ZERO, const Vector2& initialAcceleration = Vector2::ZERO);

    inline bool IsDead() { return age > maxAge; };

    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    RGBA tint;
    Vector2 scale;
    float age;
    float maxAge;
};

//-----------------------------------------------------------------------------------
class ParticleEmitter
{
public:
    ParticleEmitter(const ParticleEmitterDefinition* definition, Vector2* positionToFollow);
    virtual ~ParticleEmitter() {};
    virtual void Update(float deltaSeconds);
    virtual void UpdateParticles(float deltaSeconds);
    virtual void SpawnParticles(float deltaSeconds);
    virtual void CleanUpDeadParticles();
    void CopyParticlesToMesh(Mesh* m_mesh);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    const ParticleEmitterDefinition* m_definition;
    std::vector<Particle> m_particles;
    Vector2* m_position;
    float m_emitterAge;
    float m_timeSinceLastEmission;
    float m_secondsPerParticle;
    bool m_isDead;
};

//-----------------------------------------------------------------------------------
class ParticleSystem
{
public:
    ParticleSystem(const std::string& systemName, int orderingLayer, Vector2* positionToFollow);
    ~ParticleSystem();
    void AddEmitter(ParticleEmitter* emitter);
    void Update(float deltaSeconds);

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static void DestroyImmediately(ParticleSystem* systemToDestroy);
    static void Destroy(ParticleSystem* systemToDestroy);
    static void PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerName, Vector2* followingPosition);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    ParticleSystem* next;
    ParticleSystem* prev;
    std::vector<ParticleEmitter*> m_emitters;
    const ParticleSystemDefinition* m_definition;
    int m_orderingLayer;
    bool m_isDead;
};
