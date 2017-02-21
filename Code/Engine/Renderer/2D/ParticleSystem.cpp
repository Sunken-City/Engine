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
Particle::Particle(const Vector2& spawnPosition, const ParticleEmitterDefinition* definition, float rotationDegrees /*= 0.0f*/, const Vector2& initalVelocity /*= Vector2::ZERO*/, const Vector2& initialAcceleration /*= Vector2::ZERO*/, const RGBA& color /*= RGBA::WHITE*/) 
    : m_position(spawnPosition)
    , m_velocity(initalVelocity)
    , m_acceleration(initialAcceleration)
    , m_rotationDegrees(rotationDegrees)
    , m_age(0.0f)
    , m_color(color)
{
    m_scale = definition->m_properties.Get<Range<Vector2>>(PROPERTY_INITIAL_SCALE).GetRandom();
    m_maxAge = definition->m_properties.Get<Range<float>>(PROPERTY_PARTICLE_LIFETIME).GetRandom();
    m_angularVelocityDegrees = definition->m_properties.Get<Range<float>>(PROPERTY_INITIAL_ANGULAR_VELOCITY_DEGREES).GetRandom();

    Range<float> explosiveVelocityForce = 0.0f;
    definition->m_properties.Get<Range<float>>(PROPERTY_EXPLOSIVE_VELOCITY_MAGNITUDE, explosiveVelocityForce);
    m_velocity += Vector2::CreateFromPolar(explosiveVelocityForce.GetRandom(), m_rotationDegrees);
}
//-----------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(ParticleSystem* parent, const ParticleEmitterDefinition* definition, const Transform2D& startingTransform, Transform2D* parentTransform)
    : m_parentSystem(parent)
    , m_definition(definition)
    , m_emitterAge(0.0f)
    , m_timeSinceLastEmission(0.0f)
    , m_isDead(false)
    , m_transform(startingTransform)
    , m_maxEmitterAge(definition->m_properties.Get<Range<float>>(PROPERTY_MAX_EMITTER_LIFETIME).GetRandom())
    , m_particlesPerSecond(definition->m_properties.Get<float>(PROPERTY_PARTICLES_PER_SECOND))
    , m_initialNumParticlesSpawn(definition->m_properties.Get<Range<unsigned int>>(PROPERTY_INITIAL_NUM_PARTICLES).GetRandom())
{
    if (parentTransform)
    {
        m_transform.SetParent(parentTransform);
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
        }
    }
}

//-----------------------------------------------------------------------------------
ParticleEmitter::~ParticleEmitter()
{
    Flush();
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::Update(float deltaSeconds)
{
    //auto UpdateFunction = &UpdateParticles;
    if (!m_isDead)
    {
        m_emitterAge += deltaSeconds;
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
    const bool lockParticlesToEmitter = m_definition->m_properties.Get<bool>(PROPERTY_LOCK_PARTICLES_TO_EMITTER);
    const Vector2 scaleRateOfChangePerSecond = m_definition->m_properties.Get<Range<Vector2>>(PROPERTY_DELTA_SCALE_PER_SECOND).GetRandom();
    std::string debugName = m_definition->m_properties.Get<std::string>(PROPERTY_NAME);

    for (Particle& particle : m_particles)
    {
        float gravityScale = 0.0f;
        m_definition->m_properties.Get<float>("Gravity Scale", gravityScale);
        Vector2 acceleration = particle.m_acceleration + (Vector2(0.0f, -9.81f) * gravityScale);
        particle.m_position += particle.m_velocity * deltaSeconds;
        particle.m_velocity += acceleration * deltaSeconds;
        particle.m_scale += scaleRateOfChangePerSecond * deltaSeconds;
        particle.m_rotationDegrees += particle.m_angularVelocityDegrees * deltaSeconds;

        if (lockParticlesToEmitter)
        {
            particle.m_position = m_transform.GetWorldPosition();
        }

        particle.m_age += deltaSeconds;
        if (fadeoutEnabled)
        {
            float alphaAge = MathUtils::Clamp(1.0f - MathUtils::RangeMap(particle.m_age, 0.0f, particle.m_maxAge, 0.0f, 1.0f));
            float newAlpha = Min<float>(particle.m_color.GetAlphaFloat(), alphaAge);
            particle.m_color.SetAlphaFloat(newAlpha);
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
    Vector2 spawnPosition = m_transform.GetWorldPosition();
    Vector2 randomVectorOffset = MathUtils::GetRandomVectorInCircle(m_definition->m_properties.Get<Range<float>>(PROPERTY_SPAWN_RADIUS).GetRandom());
    float initialRotation = m_transform.GetWorldRotationDegrees() + m_definition->m_properties.Get<Range<float>>(PROPERTY_INITIAL_ROTATION_DEGREES).GetRandom();
    Vector2 initialVelocity = m_definition->m_properties.Get<Range<Vector2>>(PROPERTY_INITIAL_VELOCITY).GetRandom();
    
    spawnPosition += randomVectorOffset;
    
    RGBA color = m_parentSystem->m_colorOverride == RGBA::WHITE ? m_definition->m_properties.Get<RGBA>(PROPERTY_INITIAL_COLOR) : m_parentSystem->m_colorOverride;
    m_particles.emplace_back(spawnPosition, m_definition, initialRotation, initialVelocity, Vector2::ZERO, color);
}

//-----------------------------------------------------------------------------------
const SpriteResource* ParticleEmitter::GetSpriteResource()
{
    return m_spriteOverride ? m_spriteOverride : m_definition->m_spriteResource;
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::Flush()
{
    m_particles.clear();
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
ParticleSystem::ParticleSystem(const std::string& systemName, int orderingLayer, const Transform2D& startingTransform, Transform2D* parentTransform, const SpriteResource* spriteOverride)
    : Renderable2D(orderingLayer, true)
    , m_definition(ResourceDatabase::instance->GetParticleSystemResource(systemName))
{
    for (const ParticleEmitterDefinition* emitterDefinition : m_definition->m_emitterDefinitions)
    {
        ParticleEmitter* emitter = new ParticleEmitter(this, emitterDefinition, startingTransform, parentTransform);
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
    m_emitters.clear();
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
void ParticleSystem::PlayOneShotParticleEffect(const std::string& systemName, unsigned int const layerId, const Transform2D& startingTransform, Transform2D* parentTransform, const SpriteResource* spriteOverride)
{
    //The SpriteGameRenderer cleans up these one-shot systems whenever they're finished playing.
    ParticleSystem* newSystemToPlay = new ParticleSystem(systemName, layerId, startingTransform, parentTransform, spriteOverride);
    ASSERT_OR_DIE(newSystemToPlay->m_definition->m_type == ONE_SHOT, "Attempted to call PlayOneShotParticleEffect with a looping particle system. PlayOneShotParticleEffect is only used for one-shot particle systems.");
}

//-----------------------------------------------------------------------------------
void ParticleSystem::Flush()
{
    for (ParticleEmitter* emitter : m_emitters)
    {
        emitter->Flush();
    }
}

//-----------------------------------------------------------------------------------
void RibbonParticleSystem::Update(float deltaSeconds)
{
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

            emitter->BuildRibbonParticles(renderer);
#pragma todo("Remove this flush once we're ready to")
            renderer.FlushAndRender();
        }
    }
}

//-----------------------------------------------------------------------------------
void ParticleEmitter::BuildRibbonParticles(BufferedMeshRenderer& renderer)
{
    unsigned int numParticles = m_particles.size();
    if (numParticles == 0)
    {
        return;
    }

    float width = m_definition->m_properties.Get<float>(PROPERTY_WIDTH);
    float halfWidth = width * 0.5f;

    std::vector<RibbonParticlePiece> points;
    points.emplace_back(m_particles[0]);
    points[0].m_particle.m_position = m_transform.GetWorldPosition();
    points[0].m_particle.m_age = 0;
    for (Particle& particle : m_particles)
    {
        points.push_back(particle);
    }
    std::sort(points.begin() + 1, points.end());
    unsigned int numPoints = points.size();

    for (unsigned int i = 0; i < numPoints; ++i)
    {
        if (i == 0)
        {
            Vector2 dispFromThisToNext = points[i + 1].m_particle.m_position - points[i].m_particle.m_position;
            Vector2 perpDisp = Vector2(-dispFromThisToNext.y, dispFromThisToNext.x);
            points[i].m_perpendicularNormal = perpDisp.GetNorm();
        }
        else if (i == numPoints - 1)
        {
            Vector2 dispFromPrevToThis = points[i].m_particle.m_position - points[i - 1].m_particle.m_position;
            Vector2 perpDisp = Vector2(-dispFromPrevToThis.y, dispFromPrevToThis.x);
            points[i].m_perpendicularNormal = perpDisp.GetNorm();
        }
        else
        {
            Vector2 dispFromPrevToThis = points[i].m_particle.m_position - points[i - 1].m_particle.m_position;
            Vector2 perpDisp = Vector2(-dispFromPrevToThis.y, dispFromPrevToThis.x);
            Vector2 prevPerp = perpDisp.GetNorm();

            Vector2 dispFromThisToNext = points[i + 1].m_particle.m_position - points[i].m_particle.m_position;
            perpDisp = Vector2(-dispFromThisToNext.y, dispFromThisToNext.x);
            Vector2 nextPerp = perpDisp.GetNorm();

            points[i].m_perpendicularNormal = Lerp<Vector2>(prevPerp, nextPerp, 0.5f);
            points[i].m_perpendicularNormal.Normalize();
        }
    }

    for (unsigned int i = 1; i < numParticles; ++i)
    {
        Particle& particle = points[i].m_particle;
        
        //If we have a next particle, use it's color for the bottom two vertexes
        RGBA fadedColor = particle.m_color;
        if (i + 1 < numParticles)
        {
            fadedColor = points[i + 1].m_particle.m_color;
        }

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

        float agedHalfWidth = MathUtils::Clamp(halfWidth - MathUtils::RangeMap(particle.m_age, 0.0f, particle.m_maxAge, 0.0f, halfWidth));

        Vector2 bottomLeft = points[i].m_particle.m_position - (points[i].m_perpendicularNormal * agedHalfWidth);
        Vector2 bottomRight = points[i].m_particle.m_position + (points[i].m_perpendicularNormal * agedHalfWidth);
        Vector2 topLeft = points[i - 1].m_particle.m_position - (points[i - 1].m_perpendicularNormal * agedHalfWidth);
        Vector2 topRight = points[i - 1].m_particle.m_position + (points[i - 1].m_perpendicularNormal * agedHalfWidth);
        
        const SpriteResource* resource = GetSpriteResource();
        Vector2 pivotPoint = resource->m_pivotPoint;
        Vector2 uvMins = resource->m_uvBounds.mins;
        Vector2 uvMaxs = resource->m_uvBounds.maxs;
        Vector2 spriteBounds = resource->m_virtualSize;

        int startingVertex = renderer.m_builder.m_vertices.size();
        renderer.m_builder.SetColor(fadedColor);
        renderer.m_builder.SetUV(Vector2(uvMins.x, uvMaxs.y));
        renderer.m_builder.AddVertex(Vector3(bottomLeft, 0.0f));
        renderer.m_builder.SetColor(fadedColor);
        renderer.m_builder.SetUV(uvMaxs);
        renderer.m_builder.AddVertex(Vector3(bottomRight, 0.0f));
        renderer.m_builder.SetColor(particle.m_color);
        renderer.m_builder.SetUV(uvMins);
        renderer.m_builder.AddVertex(Vector3(topLeft, 0.0f));
        renderer.m_builder.SetColor(particle.m_color);
        renderer.m_builder.SetUV(Vector2(uvMaxs.x, uvMins.y));
        renderer.m_builder.AddVertex(Vector3(topRight, 0.0f));
        renderer.m_builder.AddQuadIndices(startingVertex + 1, startingVertex + 0, startingVertex + 3, startingVertex + 2);
    }

    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
}