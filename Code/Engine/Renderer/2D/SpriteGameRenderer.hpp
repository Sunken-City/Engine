#pragma once
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/DataStructures/InPlaceLinkedList.hpp"
#include "Engine/Renderer/Material.hpp"
#include <map>
#include <vector>
#include "Engine/Renderer/2D/ParticleSystem.hpp"
#include "Engine/Renderer/2D/Renderable2D.hpp"
#include "../BufferedMeshRenderer.hpp"

#define BIT_FLAG(f) (1 << (f))

class ShaderProgram;
class Mesh;
class MeshRenderer;
class Framebuffer;

//-----------------------------------------------------------------------------------
struct ViewportDefinition
{
    ViewportDefinition() {};

    uint32_t m_bottomLeftX;
    uint32_t m_bottomLeftY;
    uint32_t m_viewportWidth;
    uint32_t m_viewportHeight;
    float m_viewportAspectRatio;
    Vector2 m_cameraPosition;
};

//-----------------------------------------------------------------------------------
class SpriteLayer
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SpriteLayer(int layerIndex);
    ~SpriteLayer();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    inline void AddRenderable2D(Renderable2D* renderable) { AddInPlace(m_renderablesList, renderable); };
    inline void RemoveRenderable2D(Renderable2D* renderable) { RemoveInPlace(m_renderablesList, renderable); };
    inline void Enable() { m_isEnabled = true; }
    inline void Disable() { m_isEnabled = false; }
    inline void Toggle() { m_isEnabled = !m_isEnabled; }
    void CleanUpDeadRenderables(bool cleanUpLiveRenderables = false);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Material*> m_effectMaterials;
    AABB2 m_boundingVolume;
    Renderable2D* m_renderablesList;
    int m_layerIndex;
    float m_virtualScaleMultiplier = 1.0f;
    bool m_isEnabled;
    bool m_isCullingEnabled = true;
    bool m_isWorldSpaceLayer = true;
};

//-----------------------------------------------------------------------------------
class SpriteGameRenderer
{
public:
    //ENUMS/////////////////////////////////////////////////////////////////////
    enum class Alignment
    {
        CENTER = 0,
        LEFT = BIT_FLAG(1),
        RIGHT = BIT_FLAG(2),
        TOP = BIT_FLAG(3),
        BOTTOM = BIT_FLAG(4),

        TOP_LEFT = TOP | LEFT,
        TOP_RIGHT = TOP | RIGHT,
        BOTTOM_LEFT = BOTTOM | LEFT,
        BOTTOM_RIGHT = BOTTOM | RIGHT,
    };

    enum class PlayerVisibility
    {
        FIRST = BIT_FLAG(0),
        SECOND = BIT_FLAG(1),
        THIRD = BIT_FLAG(2),
        FOURTH = BIT_FLAG(3),

        ALL = FIRST | SECOND | THIRD | FOURTH,
    };

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SpriteGameRenderer(const RGBA& clearColor, unsigned int widthInPixels, unsigned int heightInPixels, unsigned int importSize, float virtualSize);
    ~SpriteGameRenderer();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render();
    void RenderView(const ViewportDefinition& renderArea);
    void UpdateScreenResolution(unsigned int widthInPixels, unsigned int heightInPixels);
    void RenderLayer(SpriteLayer* layer, const ViewportDefinition& renderArea);
    void RecalculateVirtualWidthAndHeight(const ViewportDefinition& renderArea, float layerVirtualSizeScaleFactor);
    void RegisterRenderable2D(Renderable2D* renderable);
    void UnregisterRenderable2D(Renderable2D* renderable);
    SpriteLayer* CreateOrGetLayer(int layerNumber);
    void AddEffectToLayer(Material* effectMaterial, int layerNumber);
    void RemoveEffectFromLayer(Material* effectMaterial, int layerNumber);
    inline void EnableLayer(int layerNumber) { CreateOrGetLayer(layerNumber)->Enable(); };
    inline void DisableLayer(int layerNumber) { CreateOrGetLayer(layerNumber)->Disable(); };
    inline void ToggleLayer(int layerNumber) { CreateOrGetLayer(layerNumber)->Toggle(); };
    //void SortSpritesByXY(Renderable2D*& spriteList);

    //ANCHORING/////////////////////////////////////////////////////////////////////
    void UpdateAnchorTransforms();
    inline void AnchorBottomRight(Transform2D* transformToAnchor) { m_bottomRight.AddChild(transformToAnchor); };
    inline void AnchorBottomLeft(Transform2D* transformToAnchor) { m_bottomLeft.AddChild(transformToAnchor); };
    inline void AnchorTopRight(Transform2D* transformToAnchor) { m_topRight.AddChild(transformToAnchor); };
    inline void AnchorTopLeft(Transform2D* transformToAnchor) { m_topLeft.AddChild(transformToAnchor); };
    inline void RemoveAnchorBottomRight(Transform2D* transformToAnchor) { m_bottomRight.RemoveChild(transformToAnchor); };
    inline void RemoveAnchorBottomLeft(Transform2D* transformToAnchor) { m_bottomLeft.RemoveChild(transformToAnchor); };
    inline void RemoveAnchorTopRight(Transform2D* transformToAnchor) { m_topRight.RemoveChild(transformToAnchor); };
    inline void RemoveAnchorTopLeft(Transform2D* transformToAnchor) { m_topLeft.RemoveChild(transformToAnchor); };

    //SETTERS/////////////////////////////////////////////////////////////////////
    inline void SetClearColor(const RGBA& clearColor) { m_clearColor = clearColor; };
    inline void SetVirtualSize(float vsize);
    inline void SetImportSize(unsigned int importSize) { m_importSize = importSize; }; 
    inline void SetWorldBounds(const AABB2& newWorldBounds) { m_worldBounds = newWorldBounds; };
    void SetSplitscreen(unsigned int numViews = 1);
    void SetCameraPosition(const Vector2& newCameraPosition, int viewportNumber = 0);

private:
    //Set the camera's position in a set of bounds other than the world bounds.
    void UpdateCameraPositionInWorldBounds(const Vector2& newCameraPosition, float layerScale);

public:
    //GETTERS/////////////////////////////////////////////////////////////////////
    float GetPixelsPerVirtualUnit();
    float GetVirtualUnitsPerPixel();
    AABB2 GetVirtualBoundsAroundCameraCenter();
    AABB2 GetVirtualBoundsAroundWorldCenter();
    bool IsInsideWorldBounds(const Vector2& attemptedPosition);
    Vector2 GetCameraPositionInWorld();
    static PlayerVisibility GetVisibilityFilterForPlayerNumber(unsigned int i);
    
    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SpriteGameRenderer* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    ShaderProgram* m_defaultShader;
    RenderState m_defaultRenderState;
    unsigned int m_importSize; // What resolution we're authoring at, the standardized scale of the sprite in virtual space
    float m_virtualSize;
    AABB2 m_worldBounds;

private:
    Transform2D m_bottomRight;
    Transform2D m_bottomLeft;
    Transform2D m_topRight;
    Transform2D m_topLeft;
    BufferedMeshRenderer m_bufferedMeshRenderer;
    Vector2 m_screenResolution;
    Vector2 m_cameraPosition;
    float m_aspectRatio;
    std::map<int, SpriteLayer*> m_layers;
    //The box (Size in game units of our screen)
    float m_windowVirtualHeight;
    float m_windowVirtualWidth;
    float m_virtualWidth;
    float m_virtualHeight;
    Framebuffer* m_currentFBO;
    Framebuffer* m_effectFBO;
    unsigned int m_numSplitscreenViews;
    ViewportDefinition* m_viewportDefinitions;
    PlayerVisibility m_currentViewer = PlayerVisibility::FIRST;
    RGBA m_clearColor;
    
    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static const char* DEFAULT_VERT_SHADER;
    static const char* DEFAULT_FRAG_SHADER;
};

//----------------------------------------------------------------------
inline bool operator<(SpriteLayer lhs, const SpriteLayer& rhs)
{
    return lhs.m_layerIndex < rhs.m_layerIndex;
}

//-----------------------------------------------------------------------------------
inline float LowerYComparison(Sprite* first, Sprite* second)
{
    return second->m_transform.GetWorldPosition().y - first->m_transform.GetWorldPosition().y;
}