#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ParticleSystemDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Particle::Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, const Vector2& initialVelocity, const Vector2& initialAcceleration)
    : position(spawnPosition)
    , velocity(initialVelocity)
    , acceleration(initialAcceleration)
    , age(0.0f)
{
    velocity = definition->m_initialVelocity.GetRandom();
    maxAge = definition->m_lifetimePerParticle.GetRandom();
    scale = definition->m_initialScalePerParticle.GetRandom();
    tint = definition->m_initialTintPerParticle;
}

//-----------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(const ParticleEmitterDefinition* definition, Vector2* positionToFollow)
    : m_definition(definition)
    , m_position(positionToFollow)
    , m_emitterAge(0.0f)
    , m_timeSinceLastEmission(0.0f) 
    , m_isDead(false)
{
    if (definition->m_particlesPerSecond != 0.0f)
    {
        m_secondsPerParticle = 1.0f / definition->m_particlesPerSecond;
        SpawnParticles(m_secondsPerParticle * definition->m_initialNumParticlesSpawn);
    }
    else
    {
        m_secondsPerParticle = 0.0f;
        for (unsigned int i = 0; i < definition->m_initialNumParticlesSpawn; ++i)
        {
            m_particles.emplace_back(*m_position, m_definition);
        }
    }
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::Update(float deltaSeconds)
{
    if (!m_isDead)
    {
        m_emitterAge += deltaSeconds;
        UpdateParticles(deltaSeconds);
        CleanUpDeadParticles();
        SpawnParticles(deltaSeconds);
        if (m_secondsPerParticle == 0.0f && m_particles.size() == 0)
        {
            m_isDead = true;
        }
    }
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::UpdateParticles(float deltaSeconds)
{
    for (Particle& particle : m_particles)
    {
        particle.position += particle.velocity * deltaSeconds;
        particle.velocity += particle.acceleration * deltaSeconds;

        particle.age += deltaSeconds;
        particle.tint.SetAlphaFloat(MathUtils::Clamp(1.0f - MathUtils::RangeMap(particle.age, 0.0f, particle.maxAge, 0.0f, 1.0f)));
    }
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::CleanUpDeadParticles()
{
    for (size_t i = 0; i < m_particles.size();) {

        Particle& currentParticle = m_particles[i];
        if (currentParticle.IsDead()) 
        {
            m_particles[i] = m_particles[m_particles.size() - 1];
            m_particles.pop_back();
        }
        else 
        {
            ++i;
        }
    }
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::CopyParticlesToMesh(Mesh* m_mesh)
{
    unsigned int numParticles = m_particles.size();
    if (numParticles == 0)
    {
        return;
    }
    MeshBuilder builder = MeshBuilder();
    for (unsigned int i = 0; i < numParticles; ++i)
    {
        Particle& particle = m_particles[i];
        Vector2 pivotPoint = m_definition->m_spriteResource->m_pivotPoint;
        Vector2 uvMins = m_definition->m_spriteResource->m_uvBounds.mins;
        Vector2 uvMaxs = m_definition->m_spriteResource->m_uvBounds.maxs;
        Vector2 spriteBounds = m_definition->m_spriteResource->m_virtualSize;
        Matrix4x4 scale = Matrix4x4::IDENTITY;
        Matrix4x4 rotation = Matrix4x4::IDENTITY;
        Matrix4x4 translation = Matrix4x4::IDENTITY;

        Matrix4x4::MatrixMakeScale(&scale, Vector3(particle.scale, 0.0f));
        Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(0.0f));
        Matrix4x4::MatrixMakeTranslation(&translation, Vector3(particle.position, 0.0f));
        Matrix4x4 transform = scale * rotation * translation;

        Vector2 bottomLeft(-pivotPoint.x, -pivotPoint.y);
        Vector2 topRight(spriteBounds.x - pivotPoint.x, spriteBounds.y - pivotPoint.y);
        
        bottomLeft = Vector2(Vector4(bottomLeft, 0, 1) * transform);
        topRight = Vector2(Vector4(topRight, 0, 1) * transform);

        builder.AddTexturedAABB(AABB2(bottomLeft, topRight), uvMins, uvMaxs, particle.tint);
    }

    //Copy the vertices into the mesh
    m_mesh->CleanUpRenderObjects();
    builder.CopyToMesh(m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::SpawnParticles(float deltaSeconds)
{
    if (m_secondsPerParticle > 0.0f)
    {
        m_timeSinceLastEmission += deltaSeconds;
        while (m_timeSinceLastEmission >= m_secondsPerParticle)
        {
            m_particles.emplace_back(*m_position, m_definition);
            m_timeSinceLastEmission -= m_secondsPerParticle;
        }
    }
}

//-----------------------------------------------------------------------------------
ParticleSystem::ParticleSystem(const std::string& systemName, int orderingLayer, Vector2* positionToFollow) 
    : prev(nullptr)
    , next(nullptr)
    , m_orderingLayer(orderingLayer)
    , m_isDead(false)
    , m_definition(ResourceDatabase::instance->GetParticleSystemResource(systemName))
{
    for (const ParticleEmitterDefinition* emitterDefinition : m_definition->m_emitterDefinitions)
    {
        m_emitters.push_back(new ParticleEmitter(emitterDefinition, positionToFollow));
    }
    SpriteGameRenderer::instance->RegisterParticleSystem(this);
}

//-----------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem()
{
    for (ParticleEmitter* emitter : m_emitters)
    {
        delete emitter;
    }
    SpriteGameRenderer::instance->UnregisterParticleSystem(this);
}

//-----------------------------------------------------------------------------------
void ParticleSystem::Update(float deltaSeconds)
{
    if (!m_isDead)
    {
        bool areAllEmittersDead = true;
        for (ParticleEmitter* emitter : m_emitters)
        {
            emitter->Update(deltaSeconds);
            //If any of the emitters isn't dead, this will become false
            areAllEmittersDead = areAllEmittersDead && emitter->m_isDead;
        }
        m_isDead = areAllEmittersDead;
    }
}

//-----------------------------------------------------------------------------------
void ParticleSystem::DestroyImmediately(ParticleSystem* systemToDestroy)
{
    ASSERT_OR_DIE(systemToDestroy, "Attempted to delete a null ParticleSystem ptr.");
    delete systemToDestroy;
}

//-----------------------------------------------------------------------------------
void ParticleSystem::Destroy(ParticleSystem* systemToDestroy)
{
    for (ParticleEmitter* emitter : systemToDestroy->m_emitters)
    {
        emitter->m_secondsPerParticle = 0.0f;
    }
}

//-----------------------------------------------------------------------------------
void ParticleSystem::PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerId, Vector2* followingPosition)
{
    //The SpriteGameRenderer cleans up these one-shot systems whenever they're finished playing.
    ParticleSystem* newSystemToPlay = new ParticleSystem(systemName, layerId, followingPosition);
    ASSERT_OR_DIE(newSystemToPlay->m_definition->m_type == ONE_SHOT, "Attempted to call PlayOneShotParticleEffect with a looping particle system. PlayOneShotParticleEffect is only used for one-shot particle systems.");
}
