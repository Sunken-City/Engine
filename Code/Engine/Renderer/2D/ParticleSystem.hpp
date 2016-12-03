#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include <vector>
#include "Renderable2D.hpp"

class ParticleEmitterDefinition;
class ParticleSystemDefinition;

//-----------------------------------------------------------------------------------
struct Particle
{
    Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, float rotationDegrees = 0.0f, const Vector2& initalVelocity = Vector2::ZERO, const Vector2& initialAcceleration = Vector2::ZERO);

    inline bool IsDead() { return age > maxAge; };

    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    RGBA tint;
    Vector2 scale;
    float rotationDegrees;
    float age;
    float maxAge;
};

//-----------------------------------------------------------------------------------
class ParticleEmitter
{
public:
    ParticleEmitter(const ParticleEmitterDefinition* definition, Vector2* positionToFollow);
    ParticleEmitter(const ParticleEmitterDefinition* definition, Vector2 positionToSpawn, float rotationDegrees = 0.0f);
    virtual ~ParticleEmitter() {};
    virtual void Update(float deltaSeconds);
    virtual void UpdateParticles(float deltaSeconds);
    virtual void SpawnParticles(float deltaSeconds);
    virtual void CleanUpDeadParticles();
    void CopyParticlesToMesh(Mesh* m_mesh);
    void SpawnParticle();
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    const ParticleEmitterDefinition* m_definition;
    std::vector<Particle> m_particles;
    Vector2 m_position;
    Vector2* m_followablePosition = nullptr;
    float m_rotationDegrees;
    float m_emitterAge;
    float m_maxEmitterAge = FLT_MAX;
    float m_timeSinceLastEmission;
    float m_secondsPerParticle;
    bool m_isDead;
};

//-----------------------------------------------------------------------------------
class ParticleSystem : public Renderable2D
{
public:
    ParticleSystem(const std::string& systemName, int orderingLayer, Vector2* positionToFollow);
    ParticleSystem(const std::string& systemName, int orderingLayer, Vector2 positionToSpawn, float rotationDegrees = 0.0f);
    ~ParticleSystem();
    void Update(float deltaSeconds);

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static void DestroyImmediately(ParticleSystem* systemToDestroy);
    static void Destroy(ParticleSystem* systemToDestroy);
    static void PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerName, Vector2* followingPosition);
    static void PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerName, Vector2 spawnPosition, float rotationDegrees = 0.0f);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<ParticleEmitter*> m_emitters;
    const ParticleSystemDefinition* m_definition;
    bool m_isDead;
};
