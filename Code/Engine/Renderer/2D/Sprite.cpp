#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Sprite::Sprite(const std::string& resourceName, int orderingLayer, bool isEnabled) 
    : Renderable2D(orderingLayer, isEnabled)
    , m_transform()
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
    Vector2 pos = m_transform.GetWorldPosition();
    Vector2 scale = m_transform.GetWorldScale();
    const Vector2 mins((-m_spriteResource->m_pivotPoint.x) * scale.x, (-m_spriteResource->m_pivotPoint.y) * scale.y);
    const Vector2 maxs((m_spriteResource->m_virtualSize.x - m_spriteResource->m_pivotPoint.x) * scale.x, (m_spriteResource->m_virtualSize.y - m_spriteResource->m_pivotPoint.y) * scale.y);
    return AABB2(pos + mins, pos + maxs);
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
    m_material->SetIntUniform("gRecolorMode", (int)m_recolorMode);
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
    
    //Scale the bounding box
    Matrix4x4::MatrixMakeScale(&scale, Vector3(m_transform.GetWorldScale(), 0.0f));

    //Rotate the bounding box
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(m_transform.GetWorldRotationDegrees()));

    //Translate the bounding box
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(m_transform.GetWorldPosition(), 0.0f));

    //Apply our transformations
    renderer.SetModelMatrix(scale * rotation * translation);
    renderer.m_builder.AddSprite(m_spriteResource, m_tintColor);
    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
}

//-----------------------------------------------------------------------------------
AnimatedSprite::AnimatedSprite(const std::string& animationResourceName, const std::string& defaultSpriteName, int orderingLayer /*= 0*/, bool isEnabled /*= true*/)
    : Sprite(defaultSpriteName, orderingLayer, isEnabled)
    , m_currentAnimation(ResourceDatabase::instance->GetSpriteAnimationResource(animationResourceName))
{

}

//-----------------------------------------------------------------------------------
AnimatedSprite::~AnimatedSprite()
{

}

//-----------------------------------------------------------------------------------
void AnimatedSprite::Update(float deltaSeconds) 
{
    m_currentTime += deltaSeconds;
    m_spriteResource = m_currentAnimation->GetCurrentSpriteResourceAtTime(m_currentTime);
    ASSERT_OR_DIE(m_spriteResource, "Failed to get a sprite resource from the animation");
}

//-----------------------------------------------------------------------------------
const SpriteResource* SpriteAnimationResource::GetCurrentSpriteResourceAtTime(float seconds) const
{
    float currentTime = seconds;
    if (m_frames.size() < 1)
    {
        return nullptr;
    }
    if (m_loopMode == SpriteAnimationLoopMode::ONE_SHOT)
    {
        currentTime = fmodf(seconds, m_totalLengthSeconds);
        if (currentTime > m_totalLengthSeconds)
        {
            currentTime = m_totalLengthSeconds;
        }
    }
    else if (m_loopMode == SpriteAnimationLoopMode::LOOP)
    {
        if (currentTime > m_totalLengthSeconds)
        {
            currentTime = fmodf(currentTime, m_totalLengthSeconds);
        }
    }
    else if (m_loopMode == SpriteAnimationLoopMode::PING_PONG)
    {
        if (currentTime > m_totalLengthSeconds)
        {
            float animTime = fmodf(currentTime, m_totalLengthSeconds * 2.0f) - m_totalLengthSeconds;
            if (animTime > 0.0f)
            {
                currentTime = fmodf(currentTime, m_totalLengthSeconds);
            }
            else if (animTime < 0.0f)
            {
                currentTime = m_totalLengthSeconds - fmodf(currentTime, m_totalLengthSeconds);
            }
        }
    }

    const SpriteResource* currentSprite = m_frames[0].m_resource;
    for (const SpriteAnimationFrame& frame : m_frames)
    {
        if (currentTime < frame.m_timeAtFrame)
        {
            break;
        }
        else
        {
            currentSprite = frame.m_resource;
        }
    }
    return currentSprite;
}

//-----------------------------------------------------------------------------------
void SpriteAnimationResource::AddFrame(const std::string& spriteResourceName, float durationSeconds)
{
    m_frames.emplace_back(ResourceDatabase::instance->GetSpriteResource(spriteResourceName), m_totalLengthSeconds);
    m_totalLengthSeconds += durationSeconds;
}
