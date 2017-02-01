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
    , m_cameraPosition(Vector2::ZERO)
    , m_worldBounds(-Vector2::MAX, Vector2::MAX)
    , m_currentFBO(nullptr)
    , m_effectFBO(nullptr)
    , m_viewportDefinitions(nullptr)
    , m_bufferedMeshRenderer()
{
    UpdateScreenResolution(widthInPixels, heightInPixels);
    SetVirtualSize(virtualSize);
    SetSplitscreen(1);

    m_defaultShader = ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_FRAG_SHADER);
    m_blurShader = ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_BLUR_SHADER);
    m_comboShader = ShaderProgram::CreateFromShaderStrings(DEFAULT_VERT_SHADER, DEFAULT_COMBO_SHADER);

    m_blurEffect = new Material(m_blurShader, RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_comboEffect = new Material(m_comboShader, RenderState(RenderState::DepthTestingMode::OFF, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));

    Texture* currentColorTargets[2];
    currentColorTargets[0] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    currentColorTargets[1] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    Texture* currentDepthTex = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::D24S8);
    m_currentFBO = Framebuffer::FramebufferCreate(2, currentColorTargets, currentDepthTex);

    Texture* effectColorTargets[2];
    effectColorTargets[0] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    effectColorTargets[1] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    Texture* effectDepthTex = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::D24S8);
    m_effectFBO = Framebuffer::FramebufferCreate(2, effectColorTargets, effectDepthTex);

    Texture* blurColorTargets[2];
    blurColorTargets[0] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    blurColorTargets[1] = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::RGBA8);
    Texture* blurDepthTex = new Texture(widthInPixels, heightInPixels, Texture::TextureFormat::D24S8);
    m_blurFBO = Framebuffer::FramebufferCreate(2, blurColorTargets, blurDepthTex);
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
    delete m_currentFBO->m_colorTargets[0];
    delete m_currentFBO->m_colorTargets[1];
    delete m_currentFBO->m_depthStencilTarget;
    delete m_currentFBO;
    delete m_effectFBO->m_colorTargets[0];
    delete m_effectFBO->m_colorTargets[1];
    delete m_effectFBO->m_depthStencilTarget;
    delete m_blurFBO->m_colorTargets[0];
    delete m_blurFBO->m_colorTargets[1];
    delete m_blurFBO->m_depthStencilTarget;
    delete m_blurFBO;
    delete m_effectFBO;

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
    for (unsigned int i = 0; i < m_numSplitscreenViews; ++i)
    {
        m_currentViewer = GetVisibilityFilterForPlayerNumber(i);
        RenderView(m_viewportDefinitions[i]);
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RenderView(const ViewportDefinition& renderArea)
{
    m_currentFBO->Bind();
    Renderer::instance->ClearColor(m_clearColor);
    for (auto layerPair : m_layers)
    {
        RenderLayer(layerPair.second, renderArea);
    }
    //m_meshRenderer->m_material = nullptr;
    m_currentFBO->ClearColorBuffer(1, RGBA::VAPORWAVE);
    Renderer::instance->FrameBufferCopyToBack(m_currentFBO, renderArea.m_viewportWidth, renderArea.m_viewportHeight, renderArea.m_bottomLeftX, renderArea.m_bottomLeftY);
    m_currentFBO->Unbind();
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::RenderLayer(SpriteLayer* layer, const ViewportDefinition& renderArea)
{
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
        Renderer::instance->BeginOrtho(m_virtualWidth, m_virtualHeight, cameraPos);
        {
            //SortSpritesByXY(layer->m_spriteList);
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
            m_blurFBO->Bind();
            m_blurEffect->SetDiffuseTexture(m_currentFBO->m_colorTargets[1]);
            m_blurEffect->SetFloatUniform("horizontal", 0.0f);
            Renderer::instance->RenderFullScreenEffect(m_blurEffect);
            Renderer::instance->ClearDepth();

            int numPasses = 5;
            for (int i = 0; i < numPasses; ++i)
            {
                m_effectFBO->Bind();
                m_blurEffect->SetDiffuseTexture(m_blurFBO->m_colorTargets[0]);
                m_blurEffect->SetFloatUniform("horizontal", i % 2 == 0 ? 1.0f : 0.0f);
                Renderer::instance->RenderFullScreenEffect(m_blurEffect);
                Framebuffer* temporaryCurrentPointer = m_blurFBO;
                m_blurFBO = m_effectFBO;
                m_effectFBO = temporaryCurrentPointer;
                Renderer::instance->ClearDepth();
            }

            m_effectFBO->Bind();
            m_comboEffect->SetDiffuseTexture(m_currentFBO->m_colorTargets[0]);
            m_comboEffect->SetEmissiveTexture(m_blurFBO->m_colorTargets[0]);
            Renderer::instance->RenderFullScreenEffect(m_comboEffect);
            Framebuffer* temporaryCurrentPointer = m_currentFBO;
            m_currentFBO = m_effectFBO;
            m_effectFBO = temporaryCurrentPointer;
            Renderer::instance->ClearDepth();
        }

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

    int screenOffsetX = static_cast<int>(m_screenResolution.x / m_numSplitscreenViews);
    //int screenOffsetY = static_cast<int>(m_screenResolution.y / m_numSplitscreenViews);
    for (unsigned int i = 0; i < m_numSplitscreenViews; ++i)
    {
        m_viewportDefinitions[i].m_bottomLeftX = i * screenOffsetX;
        m_viewportDefinitions[i].m_bottomLeftY = 0;
        float width = (float)screenOffsetX;
        m_viewportDefinitions[i].m_viewportWidth = (uint32_t)width;
        float height = (float)m_screenResolution.y;
        m_viewportDefinitions[i].m_viewportHeight = (uint32_t)height;
        m_viewportDefinitions[i].m_viewportAspectRatio = width / height;
        m_viewportDefinitions[i].m_cameraPosition = Vector2::ZERO;
    }
}

//-----------------------------------------------------------------------------------
void SpriteGameRenderer::SetCameraPosition(const Vector2& newCameraPosition, int viewportNumber /*= 0*/)
{
    m_viewportDefinitions[viewportNumber].m_cameraPosition = newCameraPosition;
}

//-----------------------------------------------------------------------------------
Vector2 SpriteGameRenderer::GetCameraPositionInWorld()
{
    return m_cameraPosition;
}

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