#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ParticleSystemDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Particle::Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, float initialRotationDegrees, const Vector2& initialVelocity, const Vector2& initialAcceleration)
    : m_position(spawnPosition)
    , m_velocity(initialVelocity)
    , acceleration(initialAcceleration)
    , m_rotationDegrees(initialRotationDegrees)
    , m_age(0.0f)
{
    m_scale = definition->m_properties.Get<Range<Vector2>>(PROPERTY_INITIAL_SCALE).GetRandom();
    m_maxAge = definition->m_properties.Get<Range<float>>(PROPERTY_PARTICLE_LIFETIME).GetRandom();
    m_angularVelocityDegrees = definition->m_properties.Get<Range<float>>(PROPERTY_INITIAL_ANGULAR_VELOCITY_DEGREES).GetRandom();

    Range<float> explosiveVelocityForce = 0.0f;
    definition->m_properties.Get<Range<float>>(PROPERTY_EXPLOSIVE_VELOCITY_MAGNITUDE, explosiveVelocityForce);
    m_velocity += Vector2::CreateFromPolar(explosiveVelocityForce.GetRandom(), m_rotationDegrees);
}

//-----------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(ParticleSystem* parent, const ParticleEmitterDefinition* definition, Vector2* positionToFollow)
    : m_parentSystem(parent)
    , m_definition(definition)
    , m_followablePosition(positionToFollow)
    , m_rotationDegrees(0.0f)
    , m_emitterAge(0.0f)
    , m_timeSinceLastEmission(0.0f) 
    , m_isDead(false)
    , m_maxEmitterAge(definition->m_properties.Get<Range<float>>(PROPERTY_MAX_EMITTER_LIFETIME).GetRandom())
    , m_particlesPerSecond(definition->m_properties.Get<float>(PROPERTY_PARTICLES_PER_SECOND))
    , m_initialNumParticlesSpawn(definition->m_properties.Get<Range<unsigned int>>(PROPERTY_INITIAL_NUM_PARTICLES).GetRandom())
{
    if (positionToFollow)
    {
        m_position = *positionToFollow;
        m_followablePosition = positionToFollow;
    }
    if (m_particlesPerSecond != 0.0f)
    {
        m_secondsPerParticle = 1.0f / m_particlesPerSecond;
        SpawnParticles(m_secondsPerParticle * m_initialNumParticlesSpawn);
    }
    else
    {
        m_secondsPerParticle = 0.0f;
        for (unsigned int i = 0; i < m_initialNumParticlesSpawn; ++i)
        {
            SpawnParticle();
            //m_particles.emplace_back(m_position, m_definition, m_rotationDegrees);
        }
    }
}

//-----------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(ParticleSystem* parent, const ParticleEmitterDefinition* definition, Vector2 positionToSpawn, float rotationDegrees)
    : m_parentSystem(parent)
    , m_definition(definition)
    , m_emitterAge(0.0f)
    , m_timeSinceLastEmission(0.0f)
    , m_isDead(false)
    , m_rotationDegrees(rotationDegrees)
    , m_position(positionToSpawn)
    , m_followablePosition(nullptr)
    , m_maxEmitterAge(definition->m_properties.Get<Range<float>>(PROPERTY_MAX_EMITTER_LIFETIME).GetRandom())
    , m_particlesPerSecond(definition->m_properties.Get<float>(PROPERTY_PARTICLES_PER_SECOND))
    , m_initialNumParticlesSpawn(definition->m_properties.Get<Range<unsigned int>>(PROPERTY_INITIAL_NUM_PARTICLES).GetRandom())
{
    if (m_particlesPerSecond != 0.0f)
    {
        m_secondsPerParticle = 1.0f / m_particlesPerSecond;
        SpawnParticles(m_secondsPerParticle * m_initialNumParticlesSpawn);
    }
    else
    {
        m_secondsPerParticle = 0.0f;
        for (unsigned int i = 0; i < m_initialNumParticlesSpawn; ++i)
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
    //auto UpdateFunction = &UpdateParticles;
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
        if ((m_secondsPerParticle == 0.0f || (m_emitterAge > m_maxEmitterAge)) && m_particles.size() == 0)
        {
            m_isDead = true;
        }
    }
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::UpdateParticles(float deltaSeconds)
{
    const bool fadeoutEnabled = m_definition->m_properties.Get<bool>(PROPERTY_FADEOUT_ENABLED);
    const Vector2 scaleRateOfChangePerSecond = m_definition->m_properties.Get<Range<Vector2>>(PROPERTY_DELTA_SCALE_PER_SECOND).GetRandom();
    std::string debugName = m_definition->m_properties.Get<std::string>(PROPERTY_NAME);

    for (Particle& particle : m_particles)
    {
        float gravityScale = 0.0f;
        m_definition->m_properties.Get<float>("Gravity Scale", gravityScale);
        Vector2 acceleration = particle.acceleration + (Vector2(0.0f, -9.81f) * gravityScale);
        particle.m_position += particle.m_velocity * deltaSeconds;
        particle.m_velocity += acceleration * deltaSeconds;
        particle.m_scale += scaleRateOfChangePerSecond * deltaSeconds;
        particle.m_rotationDegrees += particle.m_angularVelocityDegrees * deltaSeconds;

        particle.m_age += deltaSeconds;
        if (fadeoutEnabled)
        {
            particle.m_color.SetAlphaFloat(MathUtils::Clamp(1.0f - MathUtils::RangeMap(particle.m_age, 0.0f, particle.m_maxAge, 0.0f, 1.0f)));
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
        Particle& particle = m_particles[i];
        Matrix4x4 scale = Matrix4x4::IDENTITY;
        Matrix4x4 rotation = Matrix4x4::IDENTITY;
        Matrix4x4 translation = Matrix4x4::IDENTITY;

        //Scale the bounding box
        Matrix4x4::MatrixMakeScale(&scale, Vector3(particle.m_scale, 0.0f));

        //Rotate the bounding box
        Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(particle.m_rotationDegrees));

        //Translate the bounding box
        Matrix4x4::MatrixMakeTranslation(&translation, Vector3(particle.m_position, 0.0f));

        //Apply our transformations
        Matrix4x4 transform = scale * rotation * translation;

        renderer.m_builder.AddSprite(GetSpriteResource(), particle.m_color, &transform);
    } 

    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::SpawnParticle()
{
    Vector2 spawnPosition = m_position;
    Vector2 randomVectorOffset = MathUtils::GetRandomVectorInCircle(m_definition->m_properties.Get<Range<float>>(PROPERTY_SPAWN_RADIUS).GetRandom());
    float initialRotation = m_rotationDegrees + m_definition->m_properties.Get<Range<float>>(PROPERTY_INITIAL_ROTATION_DEGREES).GetRandom();
    Vector2 initialVelocity = m_definition->m_properties.Get<Range<Vector2>>(PROPERTY_INITIAL_VELOCITY).GetRandom();
    
    spawnPosition += randomVectorOffset;

    m_particles.emplace_back(spawnPosition, m_definition, initialRotation, initialVelocity);
    m_particles.back().m_color = m_definition->m_properties.Get<RGBA>(PROPERTY_INITIAL_COLOR);
}

//-----------------------------------------------------------------------------------
const SpriteResource* ParticleEmitter::GetSpriteResource()
{
    return m_spriteOverride ? m_spriteOverride : m_definition->m_spriteResource;
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::SpawnParticles(float deltaSeconds)
{
    if (m_secondsPerParticle > 0.0f && m_emitterAge < m_maxEmitterAge && !m_parentSystem->m_isPaused)
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
        ParticleEmitter* emitter = new ParticleEmitter(this, emitterDefinition, positionToFollow);
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
        ParticleEmitter* emitter = new ParticleEmitter(this, emitterDefinition, positionToSpawn, rotationDegrees);
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

//-----------------------------------------------------------------------------------
void RibbonParticleSystem::Update(float deltaSeconds)
{
    for (ParticleEmitter* emitter : m_emitters)
    {
    }
    ParticleSystem::Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void RibbonParticleSystem::Render(BufferedMeshRenderer& renderer)
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
