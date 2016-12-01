#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include <string>
#include "Renderable2D.hpp"

class Texture;
class Material;
class BufferedMeshRender;

//-----------------------------------------------------------------------------------
class SpriteResource
{
public:
    SpriteResource() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    AABB2 GetDefaultBounds() const;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Texture* m_texture;
    AABB2 m_uvBounds;
    Vector2 m_pixelSize;
    Vector2 m_virtualSize;
    Vector2 m_pivotPoint; //Center of Rotation and Scale (basically the origin of this sprite in local space)
    Material* m_defaultMaterial;

private:
    //Prevent copy by value for these resources.
    SpriteResource(const SpriteResource&) = delete;
};

//-----------------------------------------------------------------------------------
class Sprite : public Renderable2D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Sprite(const std::string& resourceName, int orderingLayer = 0, bool isEnabled = true);
    virtual ~Sprite();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void PushSpriteToMesh(BufferedMeshRenderer& renderer);
    virtual AABB2 GetBounds();
    virtual void Render(BufferedMeshRenderer& renderer);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    const SpriteResource* m_spriteResource;
    Vector2 m_position;
    Vector2 m_scale;
    RGBA m_tintColor;
    Material* m_material;
    float m_rotationDegrees;
};