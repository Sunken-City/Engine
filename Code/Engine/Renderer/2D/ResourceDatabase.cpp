#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>

ResourceDatabase* ResourceDatabase::instance = nullptr;

//-----------------------------------------------------------------------------------
ResourceDatabase::ResourceDatabase()
{

}

//-----------------------------------------------------------------------------------
ResourceDatabase::~ResourceDatabase()
{
    for (auto resourcePair : m_spriteDatabase)
    {
        SpriteResource* resource = resourcePair.second;
        delete resource->m_defaultMaterial;
        delete resource;
    }
    for (auto resourcePair : m_particleSystemDatabase)
    {
        ParticleSystemDefinition* resource = resourcePair.second;
        delete resource;
    }
}

//-----------------------------------------------------------------------------------
void ResourceDatabase::RegisterSprite(std::string spriteName, std::string filePath)
{
    SpriteResource* resource = new SpriteResource();
    resource->m_texture = Texture::CreateOrGetTexture(filePath);
    resource->m_pixelSize = Vector2(resource->m_texture->m_texelSize);
    resource->m_uvBounds = AABB2::ZERO_TO_ONE;
    resource->m_virtualSize = (resource->m_pixelSize / static_cast<float>(SpriteGameRenderer::instance->m_importSize)) * (SpriteGameRenderer::instance->m_virtualSize);
    resource->m_pivotPoint = resource->m_virtualSize / 2.0f;
    resource->m_defaultMaterial = new Material(SpriteGameRenderer::instance->m_defaultShader, SpriteGameRenderer::instance->m_defaultRenderState);
    m_spriteDatabase[std::hash<std::string>{}(spriteName)] = resource;
}

//-----------------------------------------------------------------------------------
const SpriteResource* ResourceDatabase::GetSpriteResource(const std::string& resourceName)
{
    auto resourceIter = m_spriteDatabase.find(std::hash<std::string>{}(resourceName));
    if (resourceIter == m_spriteDatabase.end())
    {
        ERROR_AND_DIE(Stringf("Attempted to find a SpriteResource named %s, but it wasn't in the sprite database.", resourceName.c_str()));
    }
    return (*resourceIter).second;
}

//-----------------------------------------------------------------------------------
SpriteResource* ResourceDatabase::EditSpriteResource(const std::string& resourceName)
{
    auto resourceIter = m_spriteDatabase.find(std::hash<std::string>{}(resourceName));
    if (resourceIter == m_spriteDatabase.end())
    {
        ERROR_AND_DIE(Stringf("Attempted to find a SpriteResource named %s, but it wasn't in the sprite database.", resourceName.c_str()));
    }
    return (*resourceIter).second;
}

//-----------------------------------------------------------------------------------
SpriteAnimationResource* ResourceDatabase::RegisterSpriteAnimation(std::string animationName)
{
    SpriteAnimationResource* resource = new SpriteAnimationResource();
    m_spriteAnimationDatabase[std::hash<std::string>{}(animationName)] = resource;
    resource->m_name = animationName;
    return resource;
}

//-----------------------------------------------------------------------------------
const SpriteAnimationResource* ResourceDatabase::GetSpriteAnimationResource(const std::string& resourceName)
{
    auto resourceIter = m_spriteAnimationDatabase.find(std::hash<std::string>{}(resourceName));
    if (resourceIter == m_spriteAnimationDatabase.end())
    {
        ERROR_AND_DIE(Stringf("Attempted to find a ParticleSystemResource named %s, but it wasn't in the ParticleSystem database.", resourceName.c_str()));
    }
    return (*resourceIter).second;
}

//-----------------------------------------------------------------------------------
SpriteAnimationResource* ResourceDatabase::EditSpriteAnimationResource(const std::string& resourceName)
{
    auto resourceIter = m_spriteAnimationDatabase.find(std::hash<std::string>{}(resourceName));
    if (resourceIter == m_spriteAnimationDatabase.end())
    {
        ERROR_AND_DIE(Stringf("Attempted to find a ParticleSystemResource named %s, but it wasn't in the ParticleSystem database.", resourceName.c_str()));
    }
    return (*resourceIter).second;
}

//-----------------------------------------------------------------------------------
ParticleSystemDefinition* ResourceDatabase::RegisterParticleSystem(std::string particleSystemName, ParticleSystemType type)
{
    ParticleSystemDefinition* resource = new ParticleSystemDefinition(type);
    m_particleSystemDatabase[std::hash<std::string>{}(particleSystemName)] = resource;
    resource->m_name = particleSystemName;
    return resource;
}

//-----------------------------------------------------------------------------------
const ParticleSystemDefinition* ResourceDatabase::GetParticleSystemResource(const std::string& resourceName)
{
    auto resourceIter = m_particleSystemDatabase.find(std::hash<std::string>{}(resourceName));
    if (resourceIter == m_particleSystemDatabase.end())
    {
        ERROR_AND_DIE(Stringf("Attempted to find a ParticleSystemResource named %s, but it wasn't in the ParticleSystem database.", resourceName.c_str()));
    }
    return (*resourceIter).second;
}

//-----------------------------------------------------------------------------------
ParticleSystemDefinition* ResourceDatabase::EditParticleSystemResource(const std::string& resourceName)
{
    auto resourceIter = m_particleSystemDatabase.find(std::hash<std::string>{}(resourceName));
    if (resourceIter == m_particleSystemDatabase.end())
    {
        ERROR_AND_DIE(Stringf("Attempted to find a ParticleSystemResource named %s, but it wasn't in the ParticleSystem database.", resourceName.c_str()));
    }
    return (*resourceIter).second;
}
