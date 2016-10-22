#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Time/Time.hpp"

//STATIC VARIABLES/////////////////////////////////////////////////////////////////////
SpriteGameRenderer* SpriteGameRenderer::instance = nullptr;

//-----------------------------------------------------------------------------------
const char* SpriteGameRenderer::DEFAULT_VERT_SHADER =
"#version 410 core\n\
\
    uniform mat4 gModel;\
    uniform mat4 gView;\
    uniform mat4 gProj;\
    in vec2 inPosition;\
    in vec4 inColor;\
    in vec2 inUV0;\
    out vec4 passColor;\
    out vec2 passUV;\
    void main()\
    {\
        mat4 mvp = gModel * gView * gProj;\
        passUV = inUV0;\
        passColor = inColor;\
        gl_Position = vec4(inPosition, 0, 1) * mvp;\
    }";

//-----------------------------------------------------------------------------------
const char* SpriteGameRenderer::DEFAULT_FRAG_SHADER =
"#version 410 core\n\
\
    uniform sampler2D gDiffuseTexture;\
    in vec4 passColor;\
    in vec2 passUV;\
    out vec4 fragmentColor;\
    void main()\
    {\
        vec4 diffuseColor = texture(gDiffuseTexture, passUV);\
        fragmentColor = passColor * diffuseColor;\
    }";


//-----------------------------------------------------------------------------------
SpriteLayer::SpriteLayer(int layerIndex)
    : m_layer(layerIndex)
    , m_spriteList(nullptr)
    , m_particleSystemList(nullptr)
    , m_isEnabled(true)
    , m_virtualSize(SpriteGameRenderer::instance->m_virtualSize)
    , m_boundingVolume(SpriteGameRenderer::instance->m_worldBounds)
{

}

//-----------------------------------------------------------------------------------
SpriteLayer::~SpriteLayer()
{
    ParticleSystem* currentSystem = m_particleSystemList;
    while (m_particleSystemList)
    {
        if (currentSystem->next == currentSystem)
        {
            //Destroy Immediately unregisters the pointer from m_particleSystemList, selecting a new value for the head
            ParticleSystem::DestroyImmediately(currentSystem);
            break;
        }
        else
        {
            currentSystem = currentSystem->next;
            ParticleSystem::DestroyImmediately(currentSystem->prev);
        }
    }
}

//-----------------------------------------------------------------------------------
void SpriteLayer::CleanUpDeadParticleSystems()
{
    ParticleSystem* currentSystem = m_particleSystemList;
    if (currentSystem)
    {
        do
        {
            if (currentSystem->m_isDead)
            {
                if (currentSystem->next == currentSystem)
                {
                    //Destroy Immediately unregisters the pointer from m_particleSystemList, selecting a new value for the head
                    ParticleSystem::DestroyImmediately(currentSystem);
                    break;
                }
                else
                {
                    currentSystem = currentSystem->next;
                    ParticleSystem::DestroyImmediately(currentSystem->prev);
                }
            }
            else
            {
                currentSystem = currentSystem->next;
            }
        } while (currentSystem != m_particleSystemList);
    }
}

//-----------------------------------------------------------------------------------
SpriteGameRenderer::SpriteGameRenderer(const RGBA& clearColor, unsigned int widthInPixels, unsigned int heightInPixels, unsigned int importSize, float virtualSize) : m_clearColor(clearColor)
    , m_importSize(importSize) //Artist's asset size. How big do you make the assets? 240p, 1080p, etc... (144p for gameboy zelda)
    , m_aspectRatio((float)widthInPixels/(float)heightInPixels)
    , m_defaultRenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    , m_cameraPosition(Vector2::ZERO)
    , m_worldBounds(-Vector2::MAX, Vector2::MAX)
    , m_currentFBO(nullptr)
    , m_effectFBO(nullptr)
{
    UpdateScreenResolution(widthInPixels, heightInPixels);
    SetVirtualSize(virtualSize);
    m_defaultShader = ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_FRAG_SHADER);
    m_mesh = new Mesh();
    m_meshRenderer = new MeshRenderer(m_mesh, nullptr);

    Texture* currentColorTarget = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    Texture* currentDepthTex = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::D24S8);
    m_currentFBO = Framebuffer::FramebufferCreate(1, &currentColorTarget, currentDepthTex);

    Texture* effectColorTarget = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    Texture* effectDepthTex = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::D24S8);
    m_effectFBO = Framebuffer::FramebufferCreate(1, &effectColorTarget, effectDepthTex);
}

//-----------------------------------------------------------------------------------
SpriteGameRenderer::~SpriteGameRenderer()
{
    const bool forceDelete = true;

    delete m_defaultShader;
    delete m_mesh;
    delete m_meshRenderer;
    delete m_currentFBO->m_colorTargets[0];
    delete m_currentFBO->m_depthStencilTarget;
    delete m_currentFBO;
    delete m_effectFBO->m_colorTargets[0];
    delete m_effectFBO->m_depthStencilTarget;
    delete m_effectFBO;

    for (auto layerPair : m_layers)
    {
        SpriteLayer* layer = layerPair.second;
        layer->CleanUpDeadParticleSystems();
        delete layer;
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds);
    #pragma todo("Get changes in screen resolution for the SpriteGameRenderer")

    for (auto layerPair : m_layers)
    {
        SpriteLayer* layer = layerPair.second;
        ParticleSystem* currentSystem = layer->m_particleSystemList;
        if (currentSystem)
        {
            do
            {
                currentSystem->Update(deltaSeconds);
                currentSystem = currentSystem->next;
            } while (currentSystem != layer->m_particleSystemList);
        }
        layer->CleanUpDeadParticleSystems();
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::Render()
{
    m_currentFBO->Bind();
    Renderer::instance->ClearColor(m_clearColor);
    for (auto layerPair : m_layers)
    {
        RenderLayer(layerPair.second);
    }
    m_meshRenderer->m_material = nullptr;
    Renderer::instance->FrameBufferCopyToBack(m_currentFBO);
    m_currentFBO->Unbind();
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RenderLayer(SpriteLayer* layer)
{
    AABB2 renderBounds = GetVirtualBoundsAroundCameraCenter();
    if (layer->m_isEnabled)
    {
        Renderer::instance->BeginOrtho(m_virtualWidth, m_virtualHeight, m_cameraPosition);
        {
            SortSpritesByXY(layer->m_spriteList);
            Sprite* currentSprite = layer->m_spriteList;
            if (currentSprite)
            {
                do
                {
                    if (renderBounds.IsIntersecting(currentSprite->GetBounds()))
                    {
                        DrawSprite(currentSprite);
                    }
                    currentSprite = currentSprite->next;
                } while (currentSprite != layer->m_spriteList);
            }
            ParticleSystem* currentSystem = layer->m_particleSystemList;
            if (currentSystem)
            {
                do
                {
                    DrawParticleSystem(currentSystem);
                    currentSystem = currentSystem->next;
                } while (currentSystem != layer->m_particleSystemList);
            }
        }
        Renderer::instance->EndOrtho();

        for(Material* currentEffect : layer->m_effectMaterials) 
        {
            m_effectFBO->Bind();
            currentEffect->SetDiffuseTexture(m_currentFBO->m_colorTargets[0]);
            currentEffect->SetFloatUniform("gTime", (float)GetCurrentTimeSeconds());
            Renderer::instance->RenderFullScreenEffect(currentEffect);
            Framebuffer* temporaryCurrentPointer = m_currentFBO;
            m_currentFBO = m_effectFBO;
            m_effectFBO = temporaryCurrentPointer;
            Renderer::instance->ClearDepth();
        }
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DrawParticleSystem(ParticleSystem* system)
{
    for (ParticleEmitter* emitter : system->m_emitters)
    {
        if (emitter->m_particles.size() > 0)
        {
            emitter->m_definition->m_material->SetDiffuseTexture(emitter->m_definition->m_spriteResource->m_texture);
            m_meshRenderer->m_material = emitter->m_definition->m_material;
            m_meshRenderer->SetModelMatrix(Matrix4x4::IDENTITY);
            emitter->CopyParticlesToMesh(m_mesh);
            m_meshRenderer->Render();
        }
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RegisterParticleSystem(ParticleSystem* system)
{
    CreateOrGetLayer(system->m_orderingLayer)->AddParticleSystem(system);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UnregisterParticleSystem(ParticleSystem* system)
{
    CreateOrGetLayer(system->m_orderingLayer)->RemoveParticleSystem(system);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UpdateScreenResolution(unsigned int widthInPixels, unsigned int heightInPixels)
{
    m_screenResolution = Vector2(static_cast<float>(widthInPixels), static_cast<float>(heightInPixels));
    m_aspectRatio = m_screenResolution.x / m_screenResolution.y;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetMeshFromSprite(Sprite* sprite)
{
    unsigned int indices[6] = { 1, 2, 0, 1, 3, 2 };
    Vertex_Sprite verts[4];
    Vector2 pivotPoint = sprite->m_spriteResource->m_pivotPoint;
    Vector2 uvMins = sprite->m_spriteResource->m_uvBounds.mins;
    Vector2 uvMaxs = sprite->m_spriteResource->m_uvBounds.maxs;
    Vector2 spriteBounds = sprite->m_spriteResource->m_virtualSize;
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

    verts[0].color = sprite->m_tintColor;
    verts[1].color = sprite->m_tintColor;
    verts[2].color = sprite->m_tintColor;
    verts[3].color = sprite->m_tintColor;

    //Scale the bounding box
    Matrix4x4::MatrixMakeScale(&scale, Vector3(sprite->m_scale, 0.0f));

    //Rotate the bounding box
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(sprite->m_rotationDegrees));

    //Translate the bounding box
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(sprite->m_position, 0.0f));

    //Apply our transformations
    m_meshRenderer->SetModelMatrix(scale * rotation * translation);

    //Copy the vertices into the mesh
    m_mesh->CleanUpRenderObjects();
    m_mesh->Init(verts, 4, sizeof(Vertex_Sprite), indices, 6, &Vertex_Sprite::BindMeshToVAO);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DrawSprite(Sprite* sprite)
{
    sprite->m_material->SetDiffuseTexture(sprite->m_spriteResource->m_texture);
    m_meshRenderer->m_material = sprite->m_material;
    SetMeshFromSprite(sprite);
    m_meshRenderer->Render();
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RegisterSprite(Sprite* sprite)
{
    CreateOrGetLayer(sprite->m_orderingLayer)->AddSprite(sprite);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UnregisterSprite(Sprite* sprite)
{
    CreateOrGetLayer(sprite->m_orderingLayer)->RemoveSprite(sprite);
}

//-----------------------------------------------------------------------------------
SpriteLayer* SpriteGameRenderer::CreateOrGetLayer(int layerNumber)
{
    auto layerIter = m_layers.find(layerNumber);
    if (layerIter == m_layers.end())
    {
        SpriteLayer* newLayer = new SpriteLayer(layerNumber);
        m_layers[layerNumber] = newLayer;
        return newLayer;
    }
    else
    {
        return layerIter->second;
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::AddEffectToLayer(Material* effectMaterial, int layerNumber)
{
    CreateOrGetLayer(layerNumber)->m_effectMaterials.push_back(effectMaterial);
#pragma todo("This is going to cause a bug if we have multiple copies of the same material.")
    effectMaterial->SetFloatUniform("gStartTime", (float)GetCurrentTimeSeconds());
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RemoveEffectFromLayer(Material* effectMaterial, int layerNumber)
{
    SpriteLayer* layer = CreateOrGetLayer(layerNumber);
    if (layer)
    {
        for (auto iter = layer->m_effectMaterials.begin(); iter != layer->m_effectMaterials.end(); ++iter)
        {
            if (effectMaterial == *iter)
            {
                layer->m_effectMaterials.erase(iter);
                return;
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetVirtualSize(float vsize)
{
    m_virtualSize = vsize; 
    m_virtualHeight = m_aspectRatio < 1 ? vsize / m_aspectRatio : vsize; 
    m_virtualWidth = m_aspectRatio >= 1 ? vsize * m_aspectRatio : vsize;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetCameraPosition(const Vector2& newCameraPosition)
{
    SetCameraPositionInBounds(newCameraPosition, m_worldBounds);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetCameraPositionInBounds(const Vector2& newCameraPosition, const AABB2& otherBounds)
{
    m_cameraPosition = newCameraPosition * -1.0f;
    AABB2 cameraBounds = GetVirtualBoundsAroundCameraCenter();
    if (!otherBounds.IsPointOnOrInside(cameraBounds.mins))
    {
        Vector2 correctionVector = Vector2::CalculateCorrectionVector(cameraBounds.mins, otherBounds.mins);
        m_cameraPosition.x -= cameraBounds.mins.x < otherBounds.mins.x ? correctionVector.x : 0.0f;
        m_cameraPosition.y -= cameraBounds.mins.y < otherBounds.mins.y ? correctionVector.y : 0.0f;
    }
    if (!otherBounds.IsPointOnOrInside(cameraBounds.maxs))
    {
        Vector2 correctionVector = Vector2::CalculateCorrectionVector(cameraBounds.maxs, otherBounds.maxs);
        m_cameraPosition.x -= cameraBounds.maxs.x > otherBounds.maxs.x ? correctionVector.x : 0.0f;
        m_cameraPosition.y -= cameraBounds.maxs.y > otherBounds.maxs.y ? correctionVector.y : 0.0f;
    }
}

//-----------------------------------------------------------------------------------
Vector2 SpriteGameRenderer::GetCameraPositionInWorld()
{
    return m_cameraPosition * -1.0f;
}

//-----------------------------------------------------------------------------------
float SpriteGameRenderer::GetPixelsPerVirtualUnit()
{
    return (this->m_screenResolution.y / this->m_virtualHeight);
}

//-----------------------------------------------------------------------------------
float SpriteGameRenderer::GetVirtualUnitsPerPixel()
{
    return (this->m_virtualHeight / this->m_screenResolution.y);
}

//-----------------------------------------------------------------------------------
AABB2 SpriteGameRenderer::GetVirtualBoundsAroundCameraCenter()
{
    const Vector2 halfSize((m_virtualWidth / 2.0f), (m_virtualHeight / 2.0f));
    const Vector2 worldSpaceCameraPosition = m_cameraPosition * -1.0f;
    return AABB2(worldSpaceCameraPosition - halfSize, worldSpaceCameraPosition + halfSize);
}

//-----------------------------------------------------------------------------------
AABB2 SpriteGameRenderer::GetVirtualBoundsAroundWorldCenter()
{
    const Vector2 halfSize(m_virtualWidth / 2.0f, m_virtualHeight / 2.0f);
    return AABB2(-halfSize, halfSize);
}

//-----------------------------------------------------------------------------------
bool SpriteGameRenderer::IsInsideWorldBounds(const Vector2& attemptedPosition)
{
    return GetVirtualBoundsAroundCameraCenter().IsPointInside(attemptedPosition);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SortSpritesByXY(Sprite*& spriteList)
{
    if (spriteList)
    {
        SortInPlace(spriteList, &LowerYComparison);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(enablelayer)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("enableLayer <Layer Number>", RGBA::GRAY);
        return;
    }
    int layerNumber = args.GetIntArgument(0);
    SpriteGameRenderer::instance->EnableLayer(layerNumber);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(disablelayer)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("disableLayer <Layer Number>", RGBA::GRAY);
        return;
    }
    int layerNumber = args.GetIntArgument(0);
    SpriteGameRenderer::instance->DisableLayer(layerNumber);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(togglelayer)
{
    if (!args.HasArgs(1))
    {
        Console::instance->PrintLine("toggleLayer <Layer Number>", RGBA::GRAY);
        return;
    }
    int layerNumber = args.GetIntArgument(0);
    SpriteGameRenderer::instance->ToggleLayer(layerNumber);
}