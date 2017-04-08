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
#include "../../Math/MathUtilities.hpp"
#include "../../Input/InputSystem.hpp"
#include <gl/GL.h>
#include "../../Core/ProfilingUtils.h"
#include "../../Core/StringUtils.hpp"

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
//Based off of shader from: https://learnopengl.com/#!Advanced-Lighting/Bloom
const char* SpriteGameRenderer::DEFAULT_BLUR_SHADER =
"#version 410 core\n\
\
    uniform sampler2D gDiffuseTexture;\
    uniform float horizontal;\
    uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\
    in vec4 passColor;\
    in vec2 passUV;\
    out vec4 fragColor;\
    void main()\
    {\
        vec2 tex_offset = 1.0 / textureSize(gDiffuseTexture, 0); /*Gets size of single texel*/\
        vec3 result = texture(gDiffuseTexture, passUV).rgb * weight[0]; /*Current fragment's contribution*/\
        if (horizontal > 0.5f)\
        {\
            for (int i = 1; i < 5; ++i)\
            {\
                result += texture(gDiffuseTexture, passUV + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];\
                result += texture(gDiffuseTexture, passUV - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];\
            }\
        }\
        else\
        {\
            for (int i = 1; i < 5; ++i)\
            {\
                result += texture(gDiffuseTexture, passUV + vec2(0.0, tex_offset.y * i)).rgb * weight[i];\
                result += texture(gDiffuseTexture, passUV - vec2(0.0, tex_offset.y * i)).rgb * weight[i];\
            }\
        }\
        fragColor = vec4(pow(result, vec3(0.85)), 1.0);\
    }";

//-----------------------------------------------------------------------------------
const char* SpriteGameRenderer::DEFAULT_COMBO_SHADER =
"#version 410 core\n\
\
out vec4 fragColor;\
in vec2 passUV;\
\
uniform sampler2D gDiffuseTexture;\
uniform sampler2D gEmissiveTexture;\
\
void main()\
{\
    vec3 sceneColor = texture(gDiffuseTexture, passUV).rgb;\
    vec3 bloomColor = texture(gEmissiveTexture, passUV).rgb;\
    vec3 ceiling = max(sceneColor, bloomColor);\
    vec3 composite = sceneColor + bloomColor; /*additive blending*/\
    vec3 result = min(composite, ceiling);\
    fragColor = vec4(composite, 1.0f);\
} ";

//-----------------------------------------------------------------------------------
SpriteLayer::SpriteLayer(int layerIndex)
    : m_layerIndex(layerIndex)
    , m_renderablesList(nullptr)
    , m_isEnabled(true)
    , m_boundingVolume(SpriteGameRenderer::instance->m_worldBounds)
{

}

//-----------------------------------------------------------------------------------
SpriteLayer::~SpriteLayer()
{
    Renderable2D* currentRenderable = m_renderablesList;
    if (currentRenderable)
    {
        do
        {
            if (currentRenderable->next == currentRenderable)
            {
                delete currentRenderable;
                break;
            }
            else
            {
                currentRenderable = currentRenderable->next;
                delete currentRenderable->prev;
            }
        } while (currentRenderable != m_renderablesList);
    }
}

//-----------------------------------------------------------------------------------
void SpriteLayer::CleanUpDeadRenderables(bool cleanUpLiveRenderables)
{
    Renderable2D* currentRenderable = m_renderablesList;
    if (currentRenderable)
    {
        do
        {
            if (currentRenderable->m_isDead || cleanUpLiveRenderables)
            {
                if (currentRenderable->next == currentRenderable)
                {
                    delete currentRenderable;
                    break;
                }
                else
                {
                    currentRenderable = currentRenderable->next;
                    delete currentRenderable->prev;
                }
            }
            else
            {
                currentRenderable = currentRenderable->next;
            }
        } while (currentRenderable != m_renderablesList);
    }
}

//-----------------------------------------------------------------------------------
SpriteGameRenderer::SpriteGameRenderer(const RGBA& clearColor, unsigned int widthInPixels, unsigned int heightInPixels, unsigned int importSize, float virtualSize) : m_clearColor(clearColor)
    , m_importSize(importSize) //Artist's asset size. How big do you make the assets? 240p, 1080p, etc... (144p for gameboy zelda)
    , m_aspectRatio((float)widthInPixels/(float)heightInPixels)
    , m_defaultRenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND)
    , m_additiveBlendRenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::CULL_BACK_FACES, RenderState::BlendMode::ADDITIVE_BLEND)
    , m_cameraPosition(Vector2::ZERO)
    , m_worldBounds(-Vector2::MAX, Vector2::MAX)
    , m_currentFBO(nullptr)
    , m_fullscreenCompositeFBO(nullptr)
    , m_viewportDefinitions(nullptr)
    , m_bufferedMeshRenderer()
{
    UpdateScreenResolution(widthInPixels, heightInPixels);
    SetVirtualSize(virtualSize);
    SetSplitscreen(1);

    m_defaultShader = new ShaderProgram("Data/Shaders/default2D.vert", "Data/Shaders/default2D.frag");//ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_FRAG_SHADER);
    m_blurShader = ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_BLUR_SHADER);
    m_comboShader = ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_COMBO_SHADER);

    m_blurEffect = new Material(m_blurShader, RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_comboEffect = new Material(m_comboShader, RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_blurEffect->ReplaceSampler(Renderer::instance->CreateSampler(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP));
    m_comboEffect->ReplaceSampler(Renderer::instance->CreateSampler(GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP));
    
    Texture* currentColorTargets[2];
    currentColorTargets[0] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    currentColorTargets[1] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    Texture* effectColorTargets[2];
    effectColorTargets[0] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    effectColorTargets[1] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);

    Texture* fullscreenTargets[1];
    fullscreenTargets[0] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);

    m_fullscreenCompositeFBO = Framebuffer::FramebufferCreate(1, fullscreenTargets, nullptr);
    m_currentFBO = Framebuffer::FramebufferCreate(2, currentColorTargets, nullptr);
    m_currentFBO->FlushColorTargets();

    m_fullscreenTexturePool.AddToPool(currentColorTargets[0]);
    m_fullscreenTexturePool.AddToPool(currentColorTargets[1]);
    m_fullscreenTexturePool.AddToPool(effectColorTargets[0]);
    m_fullscreenTexturePool.AddToPool(effectColorTargets[1]);
    m_currentTexturePool = &m_fullscreenTexturePool;
}

//-----------------------------------------------------------------------------------
SpriteGameRenderer::~SpriteGameRenderer()
{
    const bool forceDelete = true;

    if (m_viewportDefinitions)
    {
        delete m_viewportDefinitions;
    }

    delete m_defaultShader;
    delete m_blurShader;
    delete m_blurEffect;
    delete m_comboShader;
    delete m_comboEffect;
    delete m_fullscreenCompositeFBO->m_colorTargets[0];
    delete m_fullscreenCompositeFBO;
    delete m_currentFBO;

    for (auto layerPair : m_layers)
    {
        SpriteLayer* layer = layerPair.second;
        layer->CleanUpDeadRenderables(forceDelete);
        delete layer;
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::Update(float deltaSeconds)
{
    #pragma todo("Get changes in screen resolution for the SpriteGameRenderer")

    for (auto layerPair : m_layers)
    {
        SpriteLayer* layer = layerPair.second;
        Renderable2D* currentRenderable = layer->m_renderablesList;
        if (currentRenderable)
        {
            do
            {
                currentRenderable->Update(deltaSeconds);
                currentRenderable = currentRenderable->next;
            } while (currentRenderable != layer->m_renderablesList);
        }
        layer->CleanUpDeadRenderables();
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::Render()
{
    m_fullscreenCompositeFBO->Bind();
    m_fullscreenCompositeFBO->ClearColorBuffer(0, RGBA::VAPORWAVE);
    m_fullscreenCompositeFBO->Unbind();

    for (unsigned int i = 0; i < m_numSplitscreenViews; ++i)
    {
        ProfilingSystem::instance->PushSample("SplitscreenViewRender");
        if (m_viewTexturePool.HasAnyTextures())
        {
            m_currentTexturePool = &m_viewTexturePool;
        }
        else
        {
            m_currentTexturePool = &m_fullscreenTexturePool;
        }
        m_currentViewer = GetVisibilityFilterForPlayerNumber(i);
        RenderView(m_viewportDefinitions[i]); 
        DampScreenshake(i);
        ProfilingSystem::instance->PopSample("SplitscreenViewRender");
    }
    m_fullscreenCompositeFBO->Bind();
    Renderer::instance->FrameBufferCopyToBack(m_fullscreenCompositeFBO, m_fullscreenCompositeFBO->m_pixelWidth, m_fullscreenCompositeFBO->m_pixelHeight);
    m_fullscreenCompositeFBO->Unbind();
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DampScreenshake(unsigned int i)
{
    m_viewportDefinitions[i].m_viewportScreenshakeMagnitude *= 0.9f;
    m_viewportDefinitions[i].m_screenshakeKnockbackBias *= 0.9f;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RenderView(const ViewportDefinition& renderArea)
{
    m_currentFBO->AddColorTarget(m_currentTexturePool->GetUnusedTexture());
    m_currentFBO->AddColorTarget(m_currentTexturePool->GetUnusedTexture());
    m_currentFBO->Bind();
    m_currentFBO->ClearColorBuffer(0, m_clearColor);
    m_currentFBO->ClearColorBuffer(1, m_clearColor);
    m_currentFBO->ClearDepthBuffer();
    CalculateScreenshakeForViewport(renderArea);

    for (auto layerPair : m_layers)
    {
        //ProfilingSystem::instance->PushSample("LayerRender");//CStringf("Layer[%i]Render", layerPair.second->m_layerIndex));
        RenderLayer(layerPair.second, renderArea);
        //ProfilingSystem::instance->PopSample("LayerRender");//CStringf("Layer[%i]Render", layerPair.second->m_layerIndex));
    }
    //m_meshRenderer->m_material = nullptr;

    if (InputSystem::instance->IsKeyDown('8'))
    {
        m_currentFBO->SwapColorTargets();
    }
    if (InputSystem::instance->IsKeyDown('9'))
    {
        Texture* tex = m_currentFBO->m_colorTargets[0];
        m_currentFBO->SwapColorTarget(m_currentTexturePool->GetUnusedTexture(), 0);
        m_currentTexturePool->ReturnToPool(tex);
    }
    if (InputSystem::instance->IsKeyDown('0'))
    {
        Texture* tex = m_currentFBO->m_colorTargets[0];
        m_currentTexturePool->ReturnToPool(m_currentTexturePool->GetUnusedTexture());
        m_currentFBO->SwapColorTarget(m_currentTexturePool->GetUnusedTexture(), 0);
        m_currentTexturePool->ReturnToPool(tex);
    }
    m_currentFBO->Bind();
    m_currentFBO->ClearColorBuffer(1, RGBA::VAPORWAVE);
    m_currentFBO->Unbind();

    m_fullscreenCompositeFBO->Bind();
    {
        float bottomLeftX = MathUtils::RangeMap((float)renderArea.m_bottomLeftX, 0, m_screenResolution.x, -1.0f, 1.0f);
        float bottomLeftY = MathUtils::RangeMap((float)renderArea.m_bottomLeftY, 0, m_screenResolution.y, -1.0f, 1.0f);
        float topRightX = MathUtils::RangeMap(static_cast<float>(renderArea.m_bottomLeftX + renderArea.m_viewportWidth), 0, m_screenResolution.x, -1.0f, 1.0f);
        float topRightY = MathUtils::RangeMap(static_cast<float>(renderArea.m_bottomLeftY + renderArea.m_viewportHeight), 0, m_screenResolution.y, -1.0f, 1.0f);

        Vector2 bottomLeft = Vector2(bottomLeftX, bottomLeftY);
        Vector2 topRight = Vector2(topRightX, topRightY);
        m_bufferedMeshRenderer.SetMaterial(Renderer::instance->m_defaultMaterial);
        m_bufferedMeshRenderer.SetDiffuseTexture(m_currentFBO->m_colorTargets[0]);
        m_bufferedMeshRenderer.SetModelMatrix(Matrix4x4::IDENTITY);
        m_bufferedMeshRenderer.m_builder.AddTexturedAABB(AABB2(bottomLeft, topRight), Vector2::ZERO, Vector2::ONE, RGBA::WHITE);
        m_bufferedMeshRenderer.m_builder.CopyToMesh(&m_bufferedMeshRenderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
        m_bufferedMeshRenderer.FlushAndRender();
    }
    m_fullscreenCompositeFBO->Unbind();

    m_bufferedMeshRenderer.m_mesh.CleanUpRenderObjects();
    Renderer::instance->m_defaultMaterial->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
    m_currentTexturePool->ReturnToPool(m_currentFBO->m_colorTargets[0]);
    m_currentTexturePool->ReturnToPool(m_currentFBO->m_colorTargets[1]);
    m_currentFBO->FlushColorTargets();
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::CalculateScreenshakeForViewport(const ViewportDefinition &renderArea)
{
    float x = GetRandomFloatInRange(-renderArea.m_viewportScreenshakeMagnitude, renderArea.m_viewportScreenshakeMagnitude);
    float y = GetRandomFloatInRange(-renderArea.m_viewportScreenshakeMagnitude, renderArea.m_viewportScreenshakeMagnitude);
    m_screenshakeOffset = Vector2(x, y) + renderArea.m_screenshakeKnockbackBias;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RenderLayer(SpriteLayer* layer, const ViewportDefinition& renderArea)
{
    static const size_t horizontalUniform = std::hash<std::string>{}("horizontal");
    static const size_t gTimeUniform = std::hash<std::string>{}("gTime");
    static const size_t gWindowResolutionUniform = std::hash<std::string>{}("gWindowResolution");

    RecalculateVirtualWidthAndHeight(renderArea, layer->m_virtualScaleMultiplier);
    UpdateCameraPositionInWorldBounds(renderArea.m_cameraPosition, layer->m_virtualScaleMultiplier);
    AABB2 renderBounds = GetVirtualBoundsAroundCameraCenter();
    UpdateAnchorTransforms();

    if (layer->m_isEnabled)
    {
        if (layer->m_isBloomEnabled)
        {
            m_currentFBO->ClearColorBuffer(1, RGBA::BLACK);
        }

        Vector2 cameraPos = layer->m_isWorldSpaceLayer ? m_cameraPosition : Vector2::ZERO;
        cameraPos += m_screenshakeOffset;
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        m_currentFBO->Bind();
        Renderer::instance->BeginOrtho(m_virtualWidth, m_virtualHeight, cameraPos);
        {
            Renderable2D* currentRenderable = layer->m_renderablesList;
            if (currentRenderable)
            {
                do
                {
                    bool canBeRendered = ((uchar)m_currentViewer & currentRenderable->m_viewableBy) > 0;
                    if (canBeRendered)
                    {
                        if (!layer->IsCullingEnabled() || !currentRenderable->IsCullable() || renderBounds.IsIntersecting(currentRenderable->GetBounds()))
                        {
                            currentRenderable->Render(m_bufferedMeshRenderer);
                        }
                    }
                    currentRenderable = currentRenderable->next;
                } while (currentRenderable != layer->m_renderablesList);
            }
        }
        Renderer::instance->EndOrtho();

        if (layer->m_isBloomEnabled)
        {
            Texture* effectCanvas = m_currentTexturePool->GetUnusedTexture();
            Texture* blurCanvas = m_currentTexturePool->GetUnusedTexture();
            Renderer::instance->SetRenderTargets(1, &blurCanvas, nullptr);

            m_blurEffect->SetDiffuseTexture(m_currentFBO->m_colorTargets[1]);
            m_blurEffect->SetFloatUniform(horizontalUniform, 0.0f);
            Renderer::instance->RenderFullScreenEffect(m_blurEffect);

            int numPasses = 6;
            for (int i = 0; i < numPasses; ++i)
            {
                Renderer::instance->SetRenderTargets(1, &effectCanvas, nullptr);
                m_blurEffect->SetDiffuseTexture(blurCanvas);
                m_blurEffect->SetFloatUniform(horizontalUniform, i % 2 == 0 ? 1.0f : 0.0f);
                Renderer::instance->RenderFullScreenEffect(m_blurEffect);

                Renderer::instance->BindFramebuffer(nullptr);
                Texture* temp = effectCanvas;
                effectCanvas = blurCanvas;
                blurCanvas = temp;
            }

            Renderer::instance->SetRenderTargets(1, &effectCanvas, nullptr);
            m_comboEffect->SetDiffuseTexture(m_currentFBO->m_colorTargets[0]);
            m_comboEffect->SetEmissiveTexture(blurCanvas);
            Renderer::instance->RenderFullScreenEffect(m_comboEffect);

            Renderer::instance->BindFramebuffer(nullptr);
            effectCanvas = m_currentFBO->SwapColorTarget(effectCanvas, 0);
            m_currentFBO->Bind();

            m_currentTexturePool->ReturnToPool(effectCanvas);
            m_currentTexturePool->ReturnToPool(blurCanvas);
        }

        Texture* effectCanvas = m_currentTexturePool->GetUnusedTexture();
        Renderer::instance->ClearDepth();
        for(const FullScreenEffect& effect: layer->m_fullScreenEffects) 
        {
            bool canBeRendered = ((uchar)m_currentViewer & effect.m_visibilityFilter) > 0;
            if (!canBeRendered)
            {
                continue;
            }

            Material* currentEffect = effect.m_material;
            Renderer::instance->SetRenderTargets(1, &effectCanvas, nullptr);
            currentEffect->SetDiffuseTexture(m_currentFBO->m_colorTargets[0]);
            currentEffect->SetFloatUniform(gTimeUniform, (float)GetCurrentTimeSeconds());
            currentEffect->SetVec2Uniform(gWindowResolutionUniform, Vector2(m_windowVirtualWidth, m_windowVirtualHeight));
            Renderer::instance->RenderFullScreenEffect(currentEffect);
            Renderer::instance->ClearDepth();
            Renderer::instance->BindFramebuffer(nullptr);
            effectCanvas = m_currentFBO->SwapColorTarget(effectCanvas, 0);
            m_currentFBO->Bind();
            m_currentFBO->ClearDepthBuffer();
        }
        m_currentTexturePool->ReturnToPool(effectCanvas);
        m_currentFBO->Bind();
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UpdateAnchorTransforms()
{
    m_bottomRight.SetPosition(Vector2(m_virtualWidth * 0.5f, m_virtualHeight * -0.5f));
    m_bottomLeft.SetPosition(Vector2(m_virtualWidth * -0.5f, m_virtualHeight * -0.5f));
    m_topRight.SetPosition(Vector2(m_virtualWidth * 0.5f, m_virtualHeight * 0.5f));
    m_topLeft.SetPosition(Vector2(m_virtualWidth * -0.5f, m_virtualHeight * 0.5f));
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RecalculateVirtualWidthAndHeight(const ViewportDefinition& renderArea, float layerVirtualSizeScaleFactor)
{
    float newVirtualWidth = m_windowVirtualHeight * renderArea.m_viewportAspectRatio;
    float newVirtualHeight = m_windowVirtualWidth / renderArea.m_viewportAspectRatio;
    m_virtualWidth = MathUtils::Lerp(0.5, m_windowVirtualWidth, newVirtualWidth) * layerVirtualSizeScaleFactor;
    m_virtualHeight = MathUtils::Lerp(0.5, m_windowVirtualHeight, newVirtualHeight) * layerVirtualSizeScaleFactor;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UpdateScreenResolution(unsigned int widthInPixels, unsigned int heightInPixels)
{
    m_screenResolution = Vector2(static_cast<float>(widthInPixels), static_cast<float>(heightInPixels));
    m_aspectRatio = m_screenResolution.x / m_screenResolution.y;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RegisterRenderable2D(Renderable2D* renderable)
{
    CreateOrGetLayer(renderable->m_orderingLayer)->AddRenderable2D(renderable);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UnregisterRenderable2D(Renderable2D* renderable)
{
    CreateOrGetLayer(renderable->m_orderingLayer)->RemoveRenderable2D(renderable);
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
void SpriteGameRenderer::AddEffectToLayer(Material* effectMaterial, int layerNumber, PlayerVisibility visibility /*= PlayerVisibility::ALL*/)
{
    FullScreenEffect fullscreenEffect(effectMaterial);
    fullscreenEffect.m_visibilityFilter = (uchar)visibility;
    CreateOrGetLayer(layerNumber)->m_fullScreenEffects.push_back(fullscreenEffect);
#pragma todo("This is going to cause a bug if we have multiple copies of the same material.")
    effectMaterial->SetFloatUniform("gStartTime", (float)GetCurrentTimeSeconds());
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RemoveEffectFromLayer(Material* effectMaterial, int layerNumber)
{
    SpriteLayer* layer = CreateOrGetLayer(layerNumber);
    if (layer)
    {
        for (auto iter = layer->m_fullScreenEffects.begin(); iter != layer->m_fullScreenEffects.end(); ++iter)
        {
            if (effectMaterial == (*iter).m_material)
            {
                layer->m_fullScreenEffects.erase(iter);
                return;
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DisableAllLayers()
{
    for (auto layerIter = m_layers.begin(); layerIter != m_layers.end(); ++layerIter)
    {
        layerIter->second->Disable();
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::EnableAllLayers()
{
    for (auto layerIter = m_layers.begin(); layerIter != m_layers.end(); ++layerIter)
    {
        layerIter->second->Enable();
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetVirtualSize(float vsize)
{
    m_virtualSize = vsize; 
    m_windowVirtualHeight = m_aspectRatio < 1 ? vsize / m_aspectRatio : vsize; 
    m_windowVirtualWidth = m_aspectRatio >= 1 ? vsize * m_aspectRatio : vsize;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::UpdateCameraPositionInWorldBounds(const Vector2& newCameraPosition, float layerScale = 1.0f)
{
    m_cameraPosition = newCameraPosition;
    AABB2 cameraBounds = GetVirtualBoundsAroundCameraCenter();
    AABB2 scaledWorldBounds = m_worldBounds * layerScale;
    if (layerScale != 1.0f)
    {
        return;
    }

    if (!scaledWorldBounds.IsPointOnOrInside(cameraBounds.mins))
    {
        Vector2 correctionVector = Vector2::CalculateCorrectionVector(cameraBounds.mins, scaledWorldBounds.mins);
        m_cameraPosition.x += cameraBounds.mins.x < scaledWorldBounds.mins.x ? correctionVector.x : 0.0f;
        m_cameraPosition.y += cameraBounds.mins.y < scaledWorldBounds.mins.y ? correctionVector.y : 0.0f;
    }
    if (!scaledWorldBounds.IsPointOnOrInside(cameraBounds.maxs))
    {
        Vector2 correctionVector = Vector2::CalculateCorrectionVector(cameraBounds.maxs, scaledWorldBounds.maxs);
        m_cameraPosition.x += cameraBounds.maxs.x > scaledWorldBounds.maxs.x ? correctionVector.x : 0.0f;
        m_cameraPosition.y += cameraBounds.maxs.y > scaledWorldBounds.maxs.y ? correctionVector.y : 0.0f;
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetSplitscreen(unsigned int numViews /*= 1*/)
{
    if (m_viewportDefinitions)
    {
        delete m_viewportDefinitions;
    }
    m_numSplitscreenViews = numViews;
    m_viewportDefinitions = new ViewportDefinition[m_numSplitscreenViews];

    int screenOffsetX = m_numSplitscreenViews == 4 ? static_cast<int>(m_screenResolution.x / 2.0f) : static_cast<int>(m_screenResolution.x / m_numSplitscreenViews);
    int screenOffsetY = m_numSplitscreenViews == 4 ? static_cast<int>(m_screenResolution.y / 2.0f) : static_cast<int>(m_screenResolution.y);
    for (unsigned int i = 0; i < m_numSplitscreenViews; ++i)
    {
        int xOffset = m_numSplitscreenViews == 4 ? (i % 2) * screenOffsetX : i * screenOffsetX;
        int yOffset = m_numSplitscreenViews == 4 ? screenOffsetY - ((i / 2) * screenOffsetY) : 0;
        m_viewportDefinitions[i].m_bottomLeftX = xOffset;
        m_viewportDefinitions[i].m_bottomLeftY = yOffset;
        float width = (float)screenOffsetX;
        m_viewportDefinitions[i].m_viewportWidth = (uint32_t)width;
        float height = (float)screenOffsetY;
        m_viewportDefinitions[i].m_viewportHeight = (uint32_t)height;
        m_viewportDefinitions[i].m_viewportAspectRatio = width / height;
        m_viewportDefinitions[i].m_cameraPosition = Vector2::ZERO;
    }

    Texture* viewColorTargets[4];
    viewColorTargets[0] = new Texture((uint32_t)screenOffsetX, (uint32_t)screenOffsetY, Texture::TextureFormat::RGBA8);
    viewColorTargets[1] = new Texture((uint32_t)screenOffsetX, (uint32_t)screenOffsetY, Texture::TextureFormat::RGBA8);
    viewColorTargets[2] = new Texture((uint32_t)screenOffsetX, (uint32_t)screenOffsetY, Texture::TextureFormat::RGBA8);
    viewColorTargets[3] = new Texture((uint32_t)screenOffsetX, (uint32_t)screenOffsetY, Texture::TextureFormat::RGBA8);

    m_viewTexturePool.FlushPool();
    m_viewTexturePool.AddToPool(viewColorTargets[0]);
    m_viewTexturePool.AddToPool(viewColorTargets[1]);
    m_viewTexturePool.AddToPool(viewColorTargets[2]);
    m_viewTexturePool.AddToPool(viewColorTargets[3]);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetCameraPosition(const Vector2& newCameraPosition, int viewportNumber /*= 0*/)
{
    m_viewportDefinitions[viewportNumber].m_cameraPosition = newCameraPosition;
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DrawVertexArray(const Vertex_Sprite* vertexes, int numVertexes, Renderer::DrawMode drawMode /*= Renderer::DrawMode::QUADS*/)
{
    if (numVertexes == 0)
    {
        return;
    }
    
    m_bufferedMeshRenderer.m_builder.Begin();
    for (int i = 0; i < numVertexes; ++i)
    {
        m_bufferedMeshRenderer.m_builder.SetColor(vertexes[i].color);
        m_bufferedMeshRenderer.m_builder.SetUV(vertexes[i].uv);
        m_bufferedMeshRenderer.m_builder.SetTBN(Vector3::ZERO, Vector3::ZERO, Vector3::ZERO);
        m_bufferedMeshRenderer.m_builder.AddVertex(Vector3(vertexes[i].position, 0.0f));
        m_bufferedMeshRenderer.m_builder.AddIndex(i);
    }
    m_bufferedMeshRenderer.m_builder.End();

    m_bufferedMeshRenderer.m_builder.CopyToMesh(&m_bufferedMeshRenderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
    m_bufferedMeshRenderer.m_mesh.m_drawMode = drawMode;
    m_bufferedMeshRenderer.SetMaterial(Renderer::instance->m_defaultMaterial);
    m_bufferedMeshRenderer.SetModelMatrix(Matrix4x4::IDENTITY);
    GL_CHECK_ERROR();
    m_bufferedMeshRenderer.FlushAndRender();
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DrawLine(const Vector2& start, const Vector2& end, const RGBA& color/* = RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
    glLineWidth(lineThickness);
    Vertex_Sprite vertexes[2];
    vertexes[0].position = start;
    vertexes[1].position = end;
    vertexes[0].color = color;
    vertexes[1].color = color;
    DrawVertexArray(vertexes, 2, Renderer::DrawMode::LINES);
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color /*= RGBA::WHITE*/)
{
    Vertex_Sprite* vertexes = new Vertex_Sprite[numSides];
    const float radiansTotal = 2.0f * MathUtils::PI;
    const float radiansPerSide = radiansTotal / numSides;
    int index = 0;

    for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
    {
        float adjustedRadians = radians + radianOffset;
        float x = center.x + (radius * cos(adjustedRadians));
        float y = center.y + (radius * sin(adjustedRadians));

        vertexes[index].color = color;
        vertexes[index++].position = Vector2(x, y);
    }
    DrawVertexArray(vertexes, numSides, Renderer::DrawMode::LINE_LOOP);
    delete[] vertexes;
}

//-----------------------------------------------------------------------------------
Vector2 SpriteGameRenderer::GetCameraPositionInWorld(int viewportNumber)
{
    if (viewportNumber == -1)
    {
        return m_cameraPosition;
    }
    else
    {
        return m_viewportDefinitions[viewportNumber].m_cameraPosition;
    }
}

//-----------------------------------------------------------------------------------
SpriteGameRenderer::PlayerVisibility SpriteGameRenderer::GetVisibilityFilterForPlayerNumber(unsigned int playerNumber)
{
    switch (playerNumber)
    {
    case (0) :
        return PlayerVisibility::FIRST;
    case (1) :
        return PlayerVisibility::SECOND;
    case (2) :
        return PlayerVisibility::THIRD;
    case (3) :
        return PlayerVisibility::FOURTH;
    }
    ERROR_AND_DIE("Passed in an invalid player number for getting the visibility filter.");
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::AddScreenshakeMagnitude(float magnitude, const Vector2& direction, int viewportNumber)
{
    SpriteGameRenderer::instance->m_viewportDefinitions[viewportNumber].m_viewportScreenshakeMagnitude += magnitude;
    SpriteGameRenderer::instance->m_viewportDefinitions[viewportNumber].m_screenshakeKnockbackBias += direction;
}

//-----------------------------------------------------------------------------------
float SpriteGameRenderer::GetPixelsPerVirtualUnit()
{
    return (this->m_screenResolution.y / this->m_windowVirtualHeight);
}

//-----------------------------------------------------------------------------------
float SpriteGameRenderer::GetVirtualUnitsPerPixel()
{
    return (this->m_windowVirtualHeight / this->m_screenResolution.y);
}

//-----------------------------------------------------------------------------------
AABB2 SpriteGameRenderer::GetVirtualBoundsAroundCameraCenter()
{
    const Vector2 halfSize((m_virtualWidth * 0.5f), (m_virtualHeight * 0.5f));
    return AABB2(m_cameraPosition - halfSize, m_cameraPosition + halfSize);
}

//-----------------------------------------------------------------------------------
AABB2 SpriteGameRenderer::GetVirtualBoundsAroundWorldCenter()
{
    const Vector2 halfSize(m_virtualWidth * 0.5f, m_virtualHeight * 0.5f);
    return AABB2(-halfSize, halfSize);
}

//-----------------------------------------------------------------------------------
bool SpriteGameRenderer::IsInsideWorldBounds(const Vector2& attemptedPosition)
{
    return GetVirtualBoundsAroundCameraCenter().IsPointInside(attemptedPosition);
}

//-----------------------------------------------------------------------------------
// void SpriteGameRenderer::SortSpritesByXY(Renderable2D*& renderableList)
// {
// //     if (renderableList)
// //     {
// //         SortInPlace(renderableList, &LowerYComparison);
// //     }
// }

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