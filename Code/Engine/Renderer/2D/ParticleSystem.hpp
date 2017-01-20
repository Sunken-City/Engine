#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include <vector>
#include "Renderable2D.hpp"
#include "../../Math/Transform2D.hpp"

class ParticleEmitterDefinition;
class ParticleSystemDefinition;
class SpriteResource;
class ParticleSystem;

//-----------------------------------------------------------------------------------
struct Particle
{
    Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, float rotationDegrees = 0.0f, const Vector2& initalVelocity = Vector2::ZERO, const Vector2& initialAcceleration = Vector2::ZERO);

    inline bool IsDead() { return m_age > m_maxAge; };

    Vector2 m_position;
    Vector2 m_velocity;
    Vector2 m_acceleration;
    Vector2 m_scale;
    float m_rotationDegrees;
    float m_angularVelocityDegrees = 0.0f;
    float m_age;
    float m_maxAge;
    RGBA m_color;
};

//-----------------------------------------------------------------------------------
class ParticleEmitter
{
public:
    ParticleEmitter(ParticleSystem* parent, const ParticleEmitterDefinition* definition, const Transform2D& startingTransform, Transform2D* parentTransform = nullptr);
    virtual ~ParticleEmitter();
    virtual void Update(float deltaSeconds);
    virtual void UpdateParticles(float deltaSeconds);
    virtual void SpawnParticles(float deltaSeconds);
    virtual void CleanUpDeadParticles();
    void BuildParticles(BufferedMeshRenderer& renderer);
    void SpawnParticle();
    const SpriteResource* GetSpriteResource();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Particle> m_particles;
    Transform2D m_transform;
    const ParticleEmitterDefinition* m_definition;
    const SpriteResource* m_spriteOverride = nullptr;
    ParticleSystem* m_parentSystem = nullptr;
    unsigned int m_initialNumParticlesSpawn;
    float m_emitterAge;
    float m_particlesPerSecond;
    float m_maxEmitterAge = FLT_MAX;
    float m_timeSinceLastEmission;
    float m_secondsPerParticle;
    bool m_isDead;
};

//-----------------------------------------------------------------------------------
class ParticleSystem : public Renderable2D
{
public:
    ParticleSystem(const std::string& systemName, int orderingLayer, const Transform2D& startingTransform, Transform2D* parentTransform = nullptr, const SpriteResource* spriteOverride = nullptr);
    virtual ~ParticleSystem();
    virtual void Update(float deltaSeconds) override;
    virtual void Render(BufferedMeshRenderer& renderer) override; 
    virtual bool IsCullable() override { return false; };
    inline void Pause() { m_isPaused = true; };
    inline void Unpause() { m_isPaused = false; };

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static void DestroyImmediately(ParticleSystem* systemToDestroy);
    static void Destroy(ParticleSystem* systemToDestroy);
    static void PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerName, const Transform2D& startingTransform, Transform2D* parentTransform = nullptr, const SpriteResource* spriteOverride = nullptr);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<ParticleEmitter*> m_emitters;
    const ParticleSystemDefinition* m_definition;
    bool m_isPaused = false;
};

//-----------------------------------------------------------------------------------
class RibbonParticleSystem : public ParticleSystem
{
public:
    RibbonParticleSystem(const std::string& systemName, int orderingLayer, const Transform2D& startingTransform, Transform2D* parentTransform = nullptr, const SpriteResource* spriteOverride = nullptr)
        : ParticleSystem(systemName, orderingLayer, startingTransform, parentTransform, spriteOverride)
    {};
    virtual ~RibbonParticleSystem() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds) override;
    virtual void Render(BufferedMeshRenderer& renderer) override;
};