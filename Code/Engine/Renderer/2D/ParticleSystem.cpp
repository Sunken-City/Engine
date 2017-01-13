#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ParticleSystemDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "ResourceDatabase.hpp"
#include "../../Math/Vector3.hpp"

//-----------------------------------------------------------------------------------
Particle::Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, float initialRotationDegrees, const Vector2& initialVelocity, const Vector2& initialAcceleration)
    : position(spawnPosition)
    , velocity(initialVelocity)
    , acceleration(initialAcceleration)
    , age(0.0f)
    , rotationDegrees(initialRotationDegrees)
{
    velocity = definition->m_initialVelocity.GetRandom();
    maxAge = definition->m_lifetimePerParticle.GetRandom();
    scale = definition->m_initialScalePerParticle.GetRandom();
    tint = definition->m_initialTintPerParticle;
}

//-----------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(const ParticleEmitterDefinition* definition, Vector2* positionToFollow)
    : m_definition(definition)
    , m_followablePosition(positionToFollow)
    , m_rotationDegrees(0.0f)
    , m_emitterAge(0.0f)
    , m_timeSinceLastEmission(0.0f) 
    , m_isDead(false)
    , m_maxEmitterAge(definition->m_maxLifetime.GetRandom())
{
    if (positionToFollow)
    {
        m_position = *positionToFollow;
        m_followablePosition = positionToFollow;
    }
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
            SpawnParticle();
            //m_particles.emplace_back(m_position, m_definition, m_rotationDegrees);
        }
    }
}

//-----------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(const ParticleEmitterDefinition* definition, Vector2 positionToSpawn, float rotationDegrees)
    : m_definition(definition)
    , m_emitterAge(0.0f)
    , m_timeSinceLastEmission(0.0f)
    , m_isDead(false)
    , m_rotationDegrees(rotationDegrees)
    , m_position(positionToSpawn)
    , m_followablePosition(nullptr)
    , m_maxEmitterAge(definition->m_maxLifetime.GetRandom())
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
            SpawnParticle();
        }
    }
}

//-----------------------------------------------------------------------------------
ParticleEmitter::~ParticleEmitter()
{
    m_particles.clear();
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::Update(float deltaSeconds)
{
    if (!m_isDead)
    {
        m_emitterAge += deltaSeconds;
        if (m_followablePosition)
        {
            m_position = *m_followablePosition;
        }
        UpdateParticles(deltaSeconds);
        CleanUpDeadParticles();
        SpawnParticles(deltaSeconds);
        if ((m_secondsPerParticle == 0.0f || (m_maxEmitterAge > 0.0f && m_emitterAge > m_maxEmitterAge)) && m_particles.size() == 0)
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
        float gravityScale = 0.0f;
        m_definition->m_properties.Get<float>("Gravity Scale", gravityScale);
        Vector2 acceleration = particle.acceleration + (Vector2(0.0f, -9.81f) * gravityScale);
        particle.position += particle.velocity * deltaSeconds;
        particle.velocity += acceleration * deltaSeconds;
        particle.scale += m_definition->m_scaleRateOfChangePerSecond * deltaSeconds;

        particle.age += deltaSeconds;
        if (m_definition->m_fadeoutEnabled)
        {
            particle.tint.SetAlphaFloat(MathUtils::Clamp(1.0f - MathUtils::RangeMap(particle.age, 0.0f, particle.maxAge, 0.0f, 1.0f)));
        }
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
void ParticleEmitter::BuildParticles(BufferedMeshRenderer& renderer)
{
    unsigned int numParticles = m_particles.size();
    if (numParticles == 0)
    {
        return;
    }
    MeshBuilder builder = MeshBuilder();
    std::vector<Vertex_Sprite> verts;
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < numParticles; ++i)
    {
        int currentOffset = i * 4;
        Particle& particle = m_particles[i];
        Vector2 pivotPoint = m_definition->m_spriteResource->m_pivotPoint;
        Vector2 uvMins = m_definition->m_spriteResource->m_uvBounds.mins;
        Vector2 uvMaxs = m_definition->m_spriteResource->m_uvBounds.maxs;
        Vector2 spriteBounds = m_definition->m_spriteResource->m_virtualSize;
        Matrix4x4 scale = Matrix4x4::IDENTITY;
        Matrix4x4 rotation = Matrix4x4::IDENTITY;
        Matrix4x4 translation = Matrix4x4::IDENTITY;

        //Make empty spaces in our vector.
        verts.emplace_back();
        verts.emplace_back();
        verts.emplace_back();
        verts.emplace_back();

        //Calculate the bounding box for our sprite
        //position, scale, rotation, virtual size
        verts[0 + currentOffset].position = Vector2(-pivotPoint.x, -pivotPoint.y);
        verts[1 + currentOffset].position = Vector2(spriteBounds.x - pivotPoint.x, -pivotPoint.y);
        verts[2 + currentOffset].position = Vector2(-pivotPoint.x, spriteBounds.y - pivotPoint.y);
        verts[3 + currentOffset].position = Vector2(spriteBounds.x - pivotPoint.x, spriteBounds.y - pivotPoint.y);

        //This is skewed to accomodate for STBI loading in the images the wrong way.
        verts[0 + currentOffset].uv = Vector2(uvMins.x, uvMaxs.y);
        verts[1 + currentOffset].uv = uvMaxs;
        verts[2 + currentOffset].uv = uvMins;
        verts[3 + currentOffset].uv = Vector2(uvMaxs.x, uvMins.y);

        verts[0 + currentOffset].color = particle.tint;
        verts[1 + currentOffset].color = particle.tint;
        verts[2 + currentOffset].color = particle.tint;
        verts[3 + currentOffset].color = particle.tint;

        //Scale the bounding box
        Matrix4x4::MatrixMakeScale(&scale, Vector3(particle.scale, 0.0f));

        //Rotate the bounding box
        Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(particle.rotationDegrees));

        //Translate the bounding box
        Matrix4x4::MatrixMakeTranslation(&translation, Vector3(particle.position, 0.0f));

        //Apply our transformations
        Matrix4x4 transform = scale * rotation * translation;
        verts[0 + currentOffset].position = Vector2(Vector4(verts[0 + currentOffset].position, 0, 1) * transform);
        verts[1 + currentOffset].position = Vector2(Vector4(verts[1 + currentOffset].position, 0, 1) * transform);
        verts[2 + currentOffset].position = Vector2(Vector4(verts[2 + currentOffset].position, 0, 1) * transform);
        verts[3 + currentOffset].position = Vector2(Vector4(verts[3 + currentOffset].position, 0, 1) * transform);

        //Update indicies
        indices.push_back(1 + currentOffset);
        indices.push_back(2 + currentOffset);
        indices.push_back(0 + currentOffset);
        indices.push_back(1 + currentOffset);
        indices.push_back(3 + currentOffset);
        indices.push_back(2 + currentOffset);

        renderer.m_builder.AddSprite(GetSpriteResource(), particle.tint, &transform);
    } 

    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::SpawnParticle()
{
    Vector2 spawnPosition = m_position;
    Vector2 randomVectorOffset = MathUtils::GetRandomVectorInCircle(m_definition->m_spawnRadius.GetRandom());
    spawnPosition += randomVectorOffset;
    float rotation = m_rotationDegrees + m_definition->m_initialRotationDegrees.GetRandom();
    m_particles.emplace_back(spawnPosition, m_definition, rotation);
    m_particles.back().tint = m_definition->m_initialTintPerParticle;
}

//-----------------------------------------------------------------------------------
const SpriteResource* ParticleEmitter::GetSpriteResource()
{
    return m_spriteOverride ? m_spriteOverride : m_definition->m_spriteResource;
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::SpawnParticles(float deltaSeconds)
{
    if (m_secondsPerParticle > 0.0f && m_emitterAge < m_maxEmitterAge)
    {
        m_timeSinceLastEmission += deltaSeconds;
        while (m_timeSinceLastEmission >= m_secondsPerParticle)
        {
            SpawnParticle();
            m_timeSinceLastEmission -= m_secondsPerParticle;
        }
    }
}

//-----------------------------------------------------------------------------------
ParticleSystem::ParticleSystem(const std::string& systemName, int orderingLayer, Vector2* positionToFollow, const SpriteResource* spriteOverride)
    : Renderable2D(orderingLayer, true)
    , m_definition(ResourceDatabase::instance->GetParticleSystemResource(systemName))
{
    for (const ParticleEmitterDefinition* emitterDefinition : m_definition->m_emitterDefinitions)
    {
        ParticleEmitter* emitter = new ParticleEmitter(emitterDefinition, positionToFollow);
        emitter->m_spriteOverride = spriteOverride;
        m_emitters.push_back(emitter);
    }
}

//-----------------------------------------------------------------------------------
ParticleSystem::ParticleSystem(const std::string& systemName, int orderingLayer, Vector2 positionToSpawn, float rotationDegrees, const SpriteResource* spriteOverride)
    : Renderable2D(orderingLayer, true)
    , m_definition(ResourceDatabase::instance->GetParticleSystemResource(systemName))
{
    for (const ParticleEmitterDefinition* emitterDefinition : m_definition->m_emitterDefinitions)
    {
        ParticleEmitter* emitter = new ParticleEmitter(emitterDefinition, positionToSpawn, rotationDegrees);
        emitter->m_spriteOverride = spriteOverride;
        m_emitters.push_back(emitter);
    }
}

//-----------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem()
{
    for (ParticleEmitter* emitter : m_emitters)
    {
        delete emitter;
    }
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
void ParticleSystem::Render(BufferedMeshRenderer& renderer)
{
    for (ParticleEmitter* emitter : m_emitters)
    {
        if (emitter->m_particles.size() > 0)
        {
            Texture* diffuse = emitter->m_spriteOverride ? emitter->m_spriteOverride->m_texture : emitter->m_definition->m_spriteResource->m_texture;
            emitter->m_definition->m_material->SetDiffuseTexture(diffuse);
            renderer.SetMaterial(emitter->m_definition->m_material);
            renderer.SetModelMatrix(Matrix4x4::IDENTITY);
            emitter->BuildParticles(renderer);
#pragma todo("Remove this flush once we're ready to")
            renderer.FlushAndRender();
        }
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
void ParticleSystem::PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerId, Vector2* followingPosition, const SpriteResource* spriteOverride)
{
    //The SpriteGameRenderer cleans up these one-shot systems whenever they're finished playing.
    ParticleSystem* newSystemToPlay = new ParticleSystem(systemName, layerId, followingPosition, spriteOverride);
    ASSERT_OR_DIE(newSystemToPlay->m_definition->m_type == ONE_SHOT, "Attempted to call PlayOneShotParticleEffect with a looping particle system. PlayOneShotParticleEffect is only used for one-shot particle systems.");
}

//-----------------------------------------------------------------------------------
void ParticleSystem::PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerId, Vector2 spawnPosition, float rotationDegrees, const SpriteResource* spriteOverride)
{
    //The SpriteGameRenderer cleans up these one-shot systems whenever they're finished playing.
    ParticleSystem* newSystemToPlay = new ParticleSystem(systemName, layerId, spawnPosition, rotationDegrees, spriteOverride);
    ASSERT_OR_DIE(newSystemToPlay->m_definition->m_type == ONE_SHOT, "Attempted to call PlayOneShotParticleEffect with a looping particle system. PlayOneShotParticleEffect is only used for one-shot particle systems.");
}
