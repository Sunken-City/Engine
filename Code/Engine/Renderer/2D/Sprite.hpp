#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include <string>

class Texture;
class Material;

class SpriteResource
{
public:
    SpriteResource() {};
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

class Sprite
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Sprite(const std::string& resourceName, int orderingLayer = 0, bool isEnabled = true);
    ~Sprite();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void ChangeLayer(int layer);
    void Enable();
    void Disable();
    AABB2 GetBounds();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    const SpriteResource* m_spriteResource;
    int m_orderingLayer; //Drawing order is ordered by layer, smallest to largest
    Vector2 m_position;
    Vector2 m_scale;
    float m_rotationDegrees;
    RGBA m_tintColor;
    bool m_isEnabled; //If disabled - does not get rendered
    Material* m_material;
    Sprite* prev;
    Sprite* next;
};