#pragma once
#include <string>
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "UniformBuffer.hpp"

//-----------------------------------------------------------------------------------
class AABB2;
class AABB3;
class Texture;
class Face;
class BitmapFont;
class ShaderProgram;
class Material;
class MeshRenderer;
class Framebuffer;
class TextBox;
struct Vertex_PCT;
struct Vertex_PCUTB;

#define GL_CHECK_ERROR()

class Renderer
{
public:
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum class DrawMode : unsigned int
    {
        QUADS,
        QUAD_STRIP,
        POINTS,
        LINES,
        LINE_LOOP,
        POLYGON,
        TRIANGLES,
        TRIANGLE_STRIP,
        NUM_DRAW_MODES
    };

    //TYPEDEFS//////////////////////////////////////////////////////////////////////////
    typedef unsigned int GLuint;
    typedef int GLint;
    typedef int GLsizei;
    typedef unsigned int GLenum;
    typedef bool GLboolean;


    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Renderer(const Vector2Int& windowSize);
    virtual ~Renderer();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    virtual void CreateDefaultResources() = 0;
    virtual void ClearScreen(float red, float green, float blue) = 0;
    virtual void ClearScreen(const RGBA& color) = 0;
    virtual void ClearColor(const RGBA& color) = 0;
    virtual void PushMatrix() = 0;
    virtual void PopMatrix() = 0;
    virtual void Translate(float x, float y, float z) = 0;
    virtual void Translate(const Vector2& xy) = 0;
    virtual void Translate(const Vector3& xyz) = 0;
    virtual void Rotate(float rotationDegrees) = 0;
    virtual void Rotate(float rotationDegrees, float x, float y, float z) = 0;
    virtual void Scale(float x, float y, float z) = 0;
    virtual unsigned char GetDrawMode(DrawMode mode) const = 0;

    //STATE MODIFICATION//////////////////////////////////////////////////////////////////////////
    virtual void EnableAdditiveBlending() = 0;
    virtual void EnableAlphaBlending() = 0;
    virtual void EnableInvertedBlending() = 0;
    virtual void EnableDepthTest(bool enabled) = 0;
    virtual void EnableDepthWrite() = 0;
    virtual void DisableDepthWrite() = 0;
    virtual void EnableFaceCulling(bool enabled) = 0;
    virtual void BindTexture(const Texture& texture) = 0;
    virtual void UnbindTexture() = 0;
    virtual void SetOrtho(const Vector2& bottomLeft, const Vector2& topRight) = 0;
    virtual void BeginOrtho(const Vector2& bottomLeft, const Vector2& topRight) = 0;
    virtual void BeginOrtho(float width, float height, const Vector2& cameraOffset = Vector2::ZERO) = 0;
    virtual void EndOrtho() = 0;
    virtual void SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist) = 0;
    virtual void BeginPerspective(float fovDegreesY, float aspect, float nearDist, float farDist) = 0;
    virtual void EndPerspective() = 0;
    virtual void SetViewport(GLint x, GLint y, GLsizei width, GLsizei height) = 0;
    virtual void SetPointSize(float size) = 0;
    virtual void SetLineWidth(float width) = 0;

    //BUFFERS//////////////////////////////////////////////////////////////////////////
    virtual int GenerateBufferID() = 0;
    virtual void DeleteBuffers(int vboID) = 0;
    virtual void BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts) = 0;
    virtual void BindAndBufferVBOData(int vboID, const Vertex_PCUTB* vertexes, int numVerts) = 0;
    virtual void DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode = DrawMode::TRIANGLE_STRIP) = 0;
    virtual void DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode = DrawMode::TRIANGLE_STRIP, Texture* texture = nullptr) = 0;
    virtual void DrawVBO_PCUTB(unsigned int vboID, int numVerts, DrawMode drawMode = DrawMode::TRIANGLE_STRIP, Texture* texture = nullptr) = 0;

    //DRAWING//////////////////////////////////////////////////////////////////////////
    virtual void DrawPoint(const Vector2& point, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f) = 0;
    virtual void DrawPoint(const Vector3& point, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f) = 0;
    virtual void DrawPoint(float x, float y, const RGBA& color = RGBA::WHITE, float pointSize = 1.0f) = 0;
    virtual void DrawLine(const Vector2& start, const Vector2& end, const RGBA& color = RGBA::WHITE, float lineThickness = 1.0f) = 0;
    virtual void DrawLine(const Vector3& start, const Vector3& end, const RGBA& color = RGBA::WHITE, float lineThickness = 1.0f) = 0;
    virtual void DrawAABB(const AABB2& bounds, const RGBA& color = RGBA::WHITE) = 0;
    virtual void DrawAABB(const AABB3& bounds, const RGBA& color = RGBA::WHITE) = 0;
    virtual void DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color = RGBA::WHITE) = 0;
    virtual void DrawTexturedAABB3(const AABB3& bounds, const RGBA& color = RGBA::WHITE, const Vector2& texCoordMins = Vector2::ZERO, const Vector2& texCoordMaxs = Vector2::ONE, Texture* texture = nullptr) = 0;
    virtual void DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE) = 0;
    virtual void DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture = nullptr, const RGBA& color = RGBA::WHITE) = 0;
    virtual void SetRenderTargets(size_t colorCount, Texture** inColorTargets, Texture* depthStencilTarget) = 0;
    virtual void BindFramebuffer(Framebuffer* fbo) = 0;
    virtual void FrameBufferCopyToBack(Framebuffer* fbo, uint32_t drawingWidth, uint32_t drawingHeight, uint32_t bottomLeftX = 0, uint32_t bottomLeftY = 0, int colorTargetNumber = NULL) = 0;
    virtual void RenderFullScreenEffect(Material* material) = 0;
    virtual void DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE) = 0;
    virtual void DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color = RGBA::WHITE) = 0;
    virtual void DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellWidth, float cellHeight, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr) = 0;
    virtual void DrawText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint = RGBA::WHITE, bool drawShadow = false, const BitmapFont* font = nullptr, const Vector2& right = Vector2::UNIT_X, const Vector2& up = Vector2::UNIT_Y) = 0;

    //MODERN RENDERING (AKA: ORGANIZE THESE LATER)//////////////////////////////////////////////////////////////////////////
    virtual void UnbindIbo() = 0;
    virtual void RenderBufferDestroy(GLuint buffer) = 0;                                    
    virtual GLuint GenerateVAOHandle() = 0;
    virtual GLuint RenderBufferCreate(void* data, size_t count, size_t elementSize, GLenum usage/* = GL_STATIC_DRAW*/) = 0;
    virtual int CreateSampler(GLenum min_filter, GLenum magFilter, GLenum uWrap, GLenum vWrap) = 0;
    virtual void DeleteSampler(GLuint id) = 0;
    virtual inline void PushProjection(const Matrix4x4& proj) { m_projStack.Push(proj); };
    virtual inline void PushView(const Matrix4x4& view) { m_viewStack.Push(view); };
    virtual inline void PopProjection() { m_projStack.Pop(); };
    virtual inline void PopView() { m_viewStack.Pop(); };
    virtual inline Matrix4x4 GetProjection() { return m_projStack.GetTop(); };
    virtual inline Matrix4x4 GetView() { return m_viewStack.GetTop(); };
    virtual void RotateView(float degrees, const Vector3& axis) = 0;
    virtual void TranslateView(const Vector3& translation) = 0;
    virtual void DeleteVAOHandle(GLuint m_vaoID) = 0;
    virtual void ClearDepth(float depthValue = 1.0f) = 0;
    virtual void UseShaderProgram(GLuint shaderProgramID) = 0;
    virtual GLuint CreateRenderBuffer(size_t size, void* data = nullptr) = 0;
    virtual void BindUniform(unsigned int bindPoint, UniformBuffer& buffer) = 0;

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const int CIRCLE_SIDES = 50;
    static const int HEXAGON_SIDES = 6;
    static const unsigned char plainWhiteTexel[3];
    static Renderer* instance;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    BitmapFont* m_defaultFont = nullptr;
    Texture* m_defaultTexture = nullptr;
    ShaderProgram* m_defaultShader = nullptr;
    Material* m_defaultMaterial = nullptr;
    MeshRenderer* m_fboFullScreenEffectQuad = nullptr;
    Framebuffer* m_fbo = nullptr;
    MatrixStack4x4 m_projStack;
    MatrixStack4x4 m_viewStack;
    RenderState::BlendMode m_blendMode;
    Vector2Int m_windowSize;
    bool m_faceCullingEnabled;
    bool m_depthTestingEnabled;
    bool m_depthWritingEnabled;
    float m_lineWidth = 0.0f;
    float m_pointSize = 0.0f;
    GLint m_viewportX = 0;
    GLint m_viewportY = 0;
    GLsizei m_viewportWidth = 0;
    GLsizei m_viewportHeight = 0;
    GLuint m_fboHandle = NULL;
    GLuint m_currentShaderProgramId = NULL;
};