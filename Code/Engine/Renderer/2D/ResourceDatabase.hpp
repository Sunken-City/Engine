#pragma once
#include <map>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/ParticleSystemDefinition.hpp"

class Texture;
class Material;

class ResourceDatabase
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    ResourceDatabase();
    ~ResourceDatabase();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void RegisterSprite(std::string spriteName, std::string filePath);
    const SpriteResource* GetSpriteResource(const std::string& resourceName);
    SpriteResource* EditSpriteResource(const std::string& resourceName);
    
    SpriteAnimationResource* RegisterSpriteAnimation(std::string animationName, SpriteAnimationLoopMode mode);
    const SpriteAnimationResource* GetSpriteAnimationResource(const std::string& resourceName);
    SpriteAnimationResource* EditSpriteAnimationResource(const std::string& resourceName);

    ParticleSystemDefinition* RegisterParticleSystem(std::string particleSystemName, ParticleSystemType type);
    const ParticleSystemDefinition* GetParticleSystemResource(const std::string& resourceName);
    ParticleSystemDefinition* EditParticleSystemResource(const std::string& resourceName);

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static ResourceDatabase* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<size_t, SpriteResource*> m_spriteDatabase;
    std::map<size_t, SpriteAnimationResource*> m_spriteAnimationDatabase;
    std::map<size_t, ParticleSystemDefinition*> m_particleSystemDatabase;
};