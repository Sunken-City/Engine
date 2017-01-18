#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Core/Events/NamedProperties.hpp"
#include <vector>

class Material;
class SpriteResource;

//NAMED CONSTANTS/////////////////////////////////////////////////////////////////////
static const char* PROPERTY_FADEOUT_ENABLED = "Fadeout Enabled"; 
static const char* PROPERTY_EXPLOSIVE_VELOCITY_MAGNITUDE = "Explosive Velocity Magnitude";
static const char* PROPERTY_NAME = "Name";
static const char* PROPERTY_INITIAL_NUM_PARTICLES = "Initial Number of Particles";
static const char* PROPERTY_INITIAL_SCALE = "Initial Scale";
static const char* PROPERTY_INITIAL_VELOCITY = "Initial Velocity";
static const char* PROPERTY_INITIAL_COLOR = "Initial Color";
static const char* PROPERTY_INITIAL_ANGULAR_VELOCITY_DEGREES = "Initial Angular Velocity Degrees"; 
static const char* PROPERTY_INITIAL_ROTATION_DEGREES = "Initial Rotation Degrees";
static const char* PROPERTY_PARTICLE_LIFETIME = "Particle Lifetime";
static const char* PROPERTY_MAX_EMITTER_LIFETIME = "Max Emitter Lifetime";
static const char* PROPERTY_PARTICLES_PER_SECOND = "Particles per Second";
static const char* PROPERTY_SPAWN_RADIUS = "Spawn Radius";
static const char* PROPERTY_DELTA_SCALE_PER_SECOND = "Delta Scale Per Second";

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
    mutable NamedProperties m_properties;
    const SpriteResource* m_spriteResource;
    Material* m_material;
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
    mutable NamedProperties m_properties;
    std::string m_name;
    ParticleSystemType m_type;
    ParticleSystemDefinition* next;
    ParticleSystemDefinition* prev;
};
