#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include <string>
#include <vector>
#include "Renderable2D.hpp"
#include "../../Math/Transform2D.hpp"

class Texture;
class Material;
class SpriteResource;
class BufferedMeshRender;

//-----------------------------------------------------------------------------------
enum class SpriteAnimationLoopMode
{
    ONE_SHOT,
    LOOP,
    PING_PONG,
    NUM_LOOP_MODES
};

//-----------------------------------------------------------------------------------
enum class SpriteRecolorMode
{
    NONE = 0,
    RGB = 0,
    RBG,
    GRB,
    GBR,
    BGR,
    BRG,
    LUMINENCE,
};

//-----------------------------------------------------------------------------------
struct SpriteAnimationFrame
{
    SpriteAnimationFrame(const SpriteResource* resource, float timeAtFrame) : m_resource(resource), m_timeAtFrame(timeAtFrame) {};
    const SpriteResource* m_resource = nullptr;
    float m_timeAtFrame = 0.0f;
};

//-----------------------------------------------------------------------------------
class SpriteAnimationResource
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    SpriteAnimationResource() {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    const SpriteResource* GetCurrentSpriteResourceAtTime(float seconds) const;
    void AddFrame(const std::string& spriteResourceName, float durationSeconds);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<SpriteAnimationFrame> m_frames;
    std::string m_name;
    SpriteAnimationLoopMode m_loopMode = SpriteAnimationLoopMode::ONE_SHOT;
    float m_totalLengthSeconds = 0.0f;


private:
    //Prevent copy by value for these resources.
    SpriteAnimationResource(const SpriteAnimationResource&) = delete;
};

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
    virtual void Update(float) {};
    virtual AABB2 GetBounds();
    virtual void Render(BufferedMeshRenderer& renderer);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    const SpriteResource* m_spriteResource;
    Material* m_material;
    Transform2D m_transform;
    RGBA m_tintColor;
    SpriteRecolorMode m_recolorMode = SpriteRecolorMode::NONE;
};

//-----------------------------------------------------------------------------------
class AnimatedSprite : public Sprite
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    AnimatedSprite(const std::string& animationResourceName, const std::string& defaultSpriteName, int orderingLayer = 0, bool isEnabled = true);
    virtual ~AnimatedSprite();

    virtual void Update(float deltaSeconds);

    const SpriteAnimationResource* m_currentAnimation = nullptr;
    float m_currentTime = 0.0f;
};