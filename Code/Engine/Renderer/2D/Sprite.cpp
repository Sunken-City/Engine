#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Sprite::Sprite(const std::string& resourceName, int orderingLayer, bool isEnabled) 
    : Renderable2D(orderingLayer, isEnabled)
    , m_position(Vector2::ZERO)
    , m_scale(Vector2::ONE)
    , m_rotationDegrees(0.0f)
    , m_tintColor(RGBA::WHITE)
    , m_material(nullptr)
{
    this->m_spriteResource = ResourceDatabase::instance->GetSpriteResource(resourceName);
    this->m_material = m_spriteResource->m_defaultMaterial;
}

//-----------------------------------------------------------------------------------
Sprite::~Sprite()
{
}

//-----------------------------------------------------------------------------------
AABB2 Sprite::GetBounds()
{
    const Vector2 mins((-m_spriteResource->m_pivotPoint.x) * m_scale.x, (-m_spriteResource->m_pivotPoint.y) * m_scale.y);
    const Vector2 maxs((m_spriteResource->m_virtualSize.x - m_spriteResource->m_pivotPoint.x) * m_scale.x, (m_spriteResource->m_virtualSize.y - m_spriteResource->m_pivotPoint.y) * m_scale.y);
    return AABB2(m_position + mins, m_position + maxs);
}

//-----------------------------------------------------------------------------------
AABB2 SpriteResource::GetDefaultBounds() const
{
    const Vector2 mins(-m_pivotPoint.x, -m_pivotPoint.y);
    const Vector2 maxs(m_virtualSize.x - m_pivotPoint.x, m_virtualSize.y - m_pivotPoint.y);
    return AABB2(mins, maxs);
}

//-----------------------------------------------------------------------------------
void Sprite::Render(BufferedMeshRenderer& renderer)
{
    renderer.SetMaterial(m_material);
    renderer.SetDiffuseTexture(m_spriteResource->m_texture);
    PushSpriteToMesh(renderer);
#pragma todo("This should be unneccessary once we have batching done properly")
    renderer.FlushAndRender();
}

//-----------------------------------------------------------------------------------
void Sprite::PushSpriteToMesh(BufferedMeshRenderer& renderer)
{
    unsigned int indices[6] = { 1, 2, 0, 1, 3, 2 };
    Vertex_Sprite verts[4];
    Vector2 pivotPoint = m_spriteResource->m_pivotPoint;
    Vector2 uvMins = m_spriteResource->m_uvBounds.mins;
    Vector2 uvMaxs = m_spriteResource->m_uvBounds.maxs;
    Vector2 spriteBounds = m_spriteResource->m_virtualSize;
    Matrix4x4 scale = Matrix4x4::IDENTITY;
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    Matrix4x4 translation = Matrix4x4::IDENTITY;

    //Calculate the bounding box for our sprite
    //position, scale, rotation, virtual size
    verts[0].position = Vector2(-pivotPoint.x, -pivotPoint.y);
    verts[1].position = Vector2(spriteBounds.x - pivotPoint.x, -pivotPoint.y);
    verts[2].position = Vector2(-pivotPoint.x, spriteBounds.y - pivotPoint.y);
    verts[3].position = Vector2(spriteBounds.x - pivotPoint.x, spriteBounds.y - pivotPoint.y);

    //This is skewed to accomodate for STBI loading in the images the wrong way.
    verts[0].uv = Vector2(uvMins.x, uvMaxs.y);
    verts[1].uv = uvMaxs;
    verts[2].uv = uvMins;
    verts[3].uv = Vector2(uvMaxs.x, uvMins.y);

    verts[0].color = m_tintColor;
    verts[1].color = m_tintColor;
    verts[2].color = m_tintColor;
    verts[3].color = m_tintColor;

    //Scale the bounding box
    Matrix4x4::MatrixMakeScale(&scale, Vector3(m_scale, 0.0f));

    //Rotate the bounding box
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(m_rotationDegrees));

    //Translate the bounding box
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(m_position, 0.0f));

    //Apply our transformations
    renderer.SetModelMatrix(scale * rotation * translation);

    //Copy the vertices into the mesh
    renderer.m_mesh.Init(verts, 4, sizeof(Vertex_Sprite), indices, 6, &Vertex_Sprite::BindMeshToVAO);
}
