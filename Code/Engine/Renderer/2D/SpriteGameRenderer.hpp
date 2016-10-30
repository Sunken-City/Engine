#pragma once
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/DataStructures/InPlaceLinkedList.hpp"
#include "Engine/Renderer/Material.hpp"
#include <map>
#include <vector>
#include "ParticleSystem.hpp"

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
    inline void AddSprite(Sprite* sprite) { AddInPlace(m_spriteList, sprite); };
    inline void RemoveSprite(Sprite* sprite) { RemoveInPlace(m_spriteList, sprite); };
    inline void AddParticleSystem(ParticleSystem* system) { AddInPlace(m_particleSystemList, system); };
    inline void RemoveParticleSystem(ParticleSystem* system) { RemoveInPlace(m_particleSystemList, system); };
    inline void Enable() { m_isEnabled = true; }
    inline void Disable() { m_isEnabled = false; }
    inline void Toggle() { m_isEnabled = !m_isEnabled; }
    void CleanUpDeadParticleSystems();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    int m_layer;
    Sprite* m_spriteList;
    ParticleSystem* m_particleSystemList;
    bool m_isEnabled;
    float m_virtualSize;
    AABB2 m_boundingVolume;
    std::vector<Material*> m_effectMaterials;
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

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SpriteGameRenderer(const RGBA& clearColor, unsigned int widthInPixels, unsigned int heightInPixels, unsigned int importSize, float virtualSize);
    ~SpriteGameRenderer();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render();
    void RenderView(const ViewportDefinition& renderArea);
    void UpdateScreenResolution(unsigned int widthInPixels, unsigned int heightInPixels);
    void SetMeshFromSprite(Sprite* sprite);
    void RenderLayer(SpriteLayer* layer, const ViewportDefinition& renderArea);
    void RecalculateVirtualWidthAndHeight(const ViewportDefinition& renderArea);
    void DrawParticleSystem(ParticleSystem* system);
    void RegisterParticleSystem(ParticleSystem* system);
    void UnregisterParticleSystem(ParticleSystem* system);
    void DrawSprite(Sprite* sprite);
    void RegisterSprite(Sprite* sprite);
    void UnregisterSprite(Sprite* sprite);
    SpriteLayer* CreateOrGetLayer(int layerNumber);
    void AddEffectToLayer(Material* effectMaterial, int layerNumber);
    void RemoveEffectFromLayer(Material* effectMaterial, int layerNumber);
    inline void EnableLayer(int layerNumber) { CreateOrGetLayer(layerNumber)->Enable(); };
    inline void DisableLayer(int layerNumber) { CreateOrGetLayer(layerNumber)->Disable(); };
    inline void ToggleLayer(int layerNumber) { CreateOrGetLayer(layerNumber)->Toggle(); };
    void SortSpritesByXY(Sprite*& spriteList);

    //SETTERS/////////////////////////////////////////////////////////////////////
    inline void SetClearColor(const RGBA& clearColor) { m_clearColor = clearColor; };
    inline void SetVirtualSize(float vsize);
    inline void SetImportSize(unsigned int importSize) { m_importSize = importSize; }; 
    inline void SetWorldBounds(const AABB2& newWorldBounds) { m_worldBounds = newWorldBounds; };
    void SetSplitscreen(unsigned int numViews = 1);
    void SetCameraPosition(const Vector2& newCameraPosition, int viewportNumber = 0);

private:
    //Set the camera's position in virtual coordinates.
    void UpdateCameraPosition(const Vector2& newCameraPosition);
    //Set the camera's position in a set of bounds other than the world bounds.
    void UpdateCameraPositionInBounds(const Vector2& newCameraPosition, const AABB2& otherBounds);

public:
    //GETTERS/////////////////////////////////////////////////////////////////////
    float GetPixelsPerVirtualUnit();
    float GetVirtualUnitsPerPixel();
    AABB2 GetVirtualBoundsAroundCameraCenter();
    AABB2 GetVirtualBoundsAroundWorldCenter();
    bool IsInsideWorldBounds(const Vector2& attemptedPosition);
    Vector2 GetCameraPositionInWorld();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static SpriteGameRenderer* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    ShaderProgram* m_defaultShader;
    RenderState m_defaultRenderState;
    unsigned int m_importSize; // What resolution we're authoring at, the standardized scale of the sprite in virtual space
    float m_virtualSize;
    AABB2 m_worldBounds;

private:
    RGBA m_clearColor;
    Mesh* m_mesh;
    MeshRenderer* m_meshRenderer;
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
    
    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static const char* DEFAULT_VERT_SHADER;
    static const char* DEFAULT_FRAG_SHADER;
};

//----------------------------------------------------------------------
inline bool operator<(SpriteLayer lhs, const SpriteLayer& rhs)
{
    return lhs.m_layer < rhs.m_layer;
}

//-----------------------------------------------------------------------------------
inline float LowerYComparison(Sprite* first, Sprite* second)
{
    return second->m_position.y - first->m_position.y;
}