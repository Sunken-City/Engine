#include "Engine/Renderer/OpenGL/OGLRenderer.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <math.h>

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/Renderer/AABB3.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/Face.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Memory/MemoryTracking.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Math/Vector2Int.hpp"

#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "Glu32" ) // Link in the Glu32.lib static library

extern HDC g_displayDeviceContext;
extern HGLRC g_openGLRenderingContext;

//TEMP
static GLuint gVBO = NULL;
static GLuint gVAO = NULL;
static GLuint gIBO = NULL;
static GLuint gSampler = NULL;
static GLuint gDiffuseTex = NULL;

//-----------------------------------------------------------------------------------
OGLRenderer::OGLRenderer(const Vector2Int& windowSize) 
    : Renderer(windowSize)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    HookUpOpenGLPointers();

    m_depthWritingEnabled = true;
    m_depthTestingEnabled = true;
    m_faceCullingEnabled = false;
    m_blendMode = RenderState::BlendMode::ALPHA_BLEND;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::CreateDefaultResources()
{
    m_defaultTexture = Texture::CreateTextureFromData("PlainWhite", const_cast<uchar*>(Renderer::plainWhiteTexel), 3, Vector2Int::ONE);
    m_defaultFont = BitmapFont::CreateOrGetFont("SquirrelFixedFont");
    m_defaultShader = new ShaderProgram("Data/Shaders/fixedVertexFormat.vert", "Data/Shaders/fixedVertexFormat.frag");
    m_defaultMaterial = new Material(m_defaultShader, RenderState(RenderState::DepthTestingMode::ON, RenderState::FaceCullingMode::RENDER_BACK_FACES, RenderState::BlendMode::ALPHA_BLEND));
    m_defaultMaterial->SetDiffuseTexture(m_defaultTexture);

    //Making a generic and reusable quad for full screen effects. Material gets set when rendering the effect, but the mesh is the same.
    MeshBuilder builder;
    Mesh* fboEffectMesh = new Mesh();
    builder.AddTexturedAABB(AABB2(-Vector2::ONE, Vector2::ONE), Vector2::ZERO, Vector2::ONE, RGBA::WHITE);
    builder.CopyToMesh(fboEffectMesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    m_fboFullScreenEffectQuad = new MeshRenderer(fboEffectMesh, nullptr);
}

//-----------------------------------------------------------------------------------
OGLRenderer::~OGLRenderer()
{
    delete m_defaultShader;
    delete m_defaultMaterial;
    delete m_fboFullScreenEffectQuad->m_mesh;
    delete m_fboFullScreenEffectQuad; 
    if (m_fboHandle != NULL)
    {
        glDeleteFramebuffers(1, &m_fboHandle);
    }
}

//-----------------------------------------------------------------------------------
void OGLRenderer::ClearScreen(float red, float green, float blue)
{
    glClearColor(red, green, blue, 1.f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::ClearScreen(const RGBA& color)
{
    glClearColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, 1.f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::ClearColor(const RGBA& color)
{
    glClearColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::SetOrtho(const Vector2& bottomLeft, const Vector2& topRight)
{
    glLoadIdentity();
    glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BeginOrtho(float width, float height, const Vector2& cameraOffset)
{
    Matrix4x4 translation = Matrix4x4::IDENTITY;
    Matrix4x4 projection = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeProjectionOrthogonal(&projection, width, height, 0.f, 1.f);
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(-cameraOffset.x, -cameraOffset.y, 0.0f));
    m_projStack.PushWithoutMultiply(projection);
    m_projStack.Push(translation);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BeginOrtho(const Vector2& bottomLeft, const Vector2& topRight)
{
    Matrix4x4 translation = Matrix4x4::IDENTITY;
    Matrix4x4 projection = Matrix4x4::IDENTITY;
    float width = topRight.x - bottomLeft.x;
    float height = topRight.y - bottomLeft.y;

    Matrix4x4::MatrixMakeProjectionOrthogonal(&projection, width, height, 0.f, 1.f);
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3((-width / 2.0f), (-height / 2.0f), 0.0f));
    m_projStack.PushWithoutMultiply(projection);
    m_projStack.Push(translation);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EndOrtho()
{
    //Pop the translation
    m_projStack.Pop();
    //Pop the projection matrix
    m_projStack.Pop();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::SetPerspective(float fovDegreesY, float aspect, float nearDist, float farDist)
{
    glLoadIdentity();
    gluPerspective(fovDegreesY, aspect, nearDist, farDist); //Note: absent in OpenGL ES 2.0
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BeginPerspective(float fovDegreesY, float aspect, float nearDist, float farDist)
{
    Matrix4x4 proj = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakePerspective(&proj, fovDegreesY, aspect, nearDist, farDist);
    PushProjection(proj);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EndPerspective()
{
    PopProjection();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    if (x == m_viewportX && y == m_viewportY && width == m_viewportWidth && height == m_viewportHeight)
    {
        return;
    }

    glViewport(x, y, width, height);

    m_viewportX = x;
    m_viewportY = y;
    m_viewportWidth = width;
    m_viewportHeight = height;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::RotateView(float degrees, const Vector3& axis)
{
    //From page 111 in Chapter 8 of 3D Math Primer for Graphics and Game Development
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    rotation.Rotate(degrees, axis);
    PushView(rotation);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::TranslateView(const Vector3& translation)
{
    Matrix4x4 translationMatrix = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeTranslation(&translationMatrix, translation);
    PushView(translationMatrix);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawPoint(float x, float y, const RGBA& color /*= RGBA::WHITE*/, float pointSize /*= 1.0f*/)
{
    glPointSize(pointSize);
    Vertex_PCT vertex;
    vertex.pos = Vector3(x, y, 0.0f);
    vertex.color = color;
    DrawVertexArray(&vertex, 1, DrawMode::POINTS);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawPoint(const Vector2& point, const RGBA& color /*= RGBA::WHITE*/, float pointSize /*= 1.0f*/)
{
    DrawPoint(point.x, point.y, color, pointSize);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawPoint(const Vector3& point, const RGBA& color /*= RGBA::WHITE*/, float pointSize /*= 1.0f*/)
{
    glPointSize(pointSize);
    Vertex_PCT vertex;
    vertex.pos = point;
    vertex.color = color;
    DrawVertexArray(&vertex, 1, DrawMode::POINTS);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawLine(const Vector2& start, const Vector2& end, const RGBA& color/* = RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
    //glLineWidth(lineThickness);
    Vertex_PCT vertexes[2];
    vertexes[0].pos = start;
    vertexes[1].pos = end;
    vertexes[0].color = color;
    vertexes[1].color = color;
    DrawVertexArray(vertexes, 2, DrawMode::LINES);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawLine(const Vector3& start, const Vector3& end, const RGBA& color /*= RGBA::WHITE*/, float lineThickness /*= 1.0f*/)
{
    //glLineWidth(lineThickness);
    Vertex_PCT vertexes[2];
    vertexes[0].pos = start;
    vertexes[1].pos = end;
    vertexes[0].color = color;
    vertexes[1].color = color;
    DrawVertexArray(vertexes, 2, DrawMode::LINES);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::SetPointSize(float size)
{
    if (m_pointSize == size)
    {
        return;
    }
    glPointSize(size);
    m_pointSize = size;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::SetLineWidth(float width)
{
    if (m_lineWidth == width)
    {
        return;
    }
    //glLineWidth(width);
    m_lineWidth = width;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::PushMatrix()
{
    glPushMatrix();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::PopMatrix()
{
    glPopMatrix();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::Translate(float x, float y, float z)
{
    glTranslatef(x, y, z);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::Translate(const Vector2& xy)
{
    glTranslatef(xy.x, xy.y, 0.0f);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::Translate(const Vector3& xyz)
{
    glTranslatef(xyz.x, xyz.y, xyz.z);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::Rotate(float rotationDegrees)
{
    glRotatef(rotationDegrees, 0.f, 0.f, 1.f);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::Rotate(float rotationDegrees, float x, float y, float z)
{
    glRotatef(rotationDegrees, x, y, z);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::Scale(float x, float y, float z)
{
    glScalef(x, y, z);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EnableAdditiveBlending()
{
    if (m_blendMode == RenderState::BlendMode::ADDITIVE_BLEND)
    {
        return;
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    m_blendMode = RenderState::BlendMode::ADDITIVE_BLEND;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EnableAlphaBlending()
{
    if (m_blendMode == RenderState::BlendMode::ALPHA_BLEND)
    {
        return;
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_blendMode = RenderState::BlendMode::ALPHA_BLEND;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EnableInvertedBlending()
{
    if (m_blendMode == RenderState::BlendMode::INVERTED_BLEND)
    {
        return;
    }
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    m_blendMode = RenderState::BlendMode::INVERTED_BLEND;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EnableDepthTest(bool usingDepthTest)
{
    if (m_depthTestingEnabled == usingDepthTest)
    {
        return;
    }
    usingDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    m_depthTestingEnabled = usingDepthTest;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EnableDepthWrite()
{
    if (m_depthWritingEnabled)
    {
        return;
    }
    glDepthMask(true);
    m_depthWritingEnabled = true;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DisableDepthWrite()
{
    if (!m_depthWritingEnabled)
    {
        return;
    }
    glDepthMask(false);
    m_depthWritingEnabled = false;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BindTexture(const Texture& texture)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.m_openglTextureID);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::UnbindTexture()
{
    glBindTexture(GL_TEXTURE_2D, NULL);
    glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------------
int OGLRenderer::GenerateBufferID()
{
    GLuint vboID = 0;
    glGenBuffers(1, &vboID);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferAllocation();
    #endif
    return vboID;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DeleteBuffers(int vboID)
{
    const GLuint id = ((GLuint)vboID);
    glDeleteBuffers(1, &id);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferFree();
    #endif
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BindAndBufferVBOData(int vboID, const Vertex_PCT* vertexes, int numVerts)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_PCT) * numVerts, vertexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BindAndBufferVBOData(int vboID, const Vertex_PCUTB* vertexes, int numVerts)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex_PCUTB) * numVerts, vertexes, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawVertexArray(const Vertex_PCT* vertexes, int numVertexes, DrawMode drawMode /*= DrawMode::TRIANGLE_STRIP*/)
{
    if (numVertexes == 0)
    {
        return;
    }
    MeshBuilder builder;
    builder.Begin();
    for (int i = 0; i < numVertexes; ++i)
    {
        builder.SetColor(vertexes[i].color);
        builder.SetUV(vertexes[i].texCoords);
        builder.SetTBN(Vector3::ZERO, Vector3::ZERO, Vector3::ZERO);
        builder.AddVertex(vertexes[i].pos);
        builder.AddIndex(i);
    }
    builder.End();

    Mesh* mesh = new Mesh();
    builder.CopyToMesh(mesh, &Vertex_PCT::Copy, sizeof(Vertex_PCT), &Vertex_PCT::BindMeshToVAO);
    mesh->m_drawMode = drawMode;
    MeshRenderer* thingToRender = new MeshRenderer(mesh, m_defaultMaterial);
    m_defaultMaterial->SetMatrices(Matrix4x4::IDENTITY, m_viewStack.GetTop(), m_projStack.GetTop());
    GL_CHECK_ERROR();
    thingToRender->Render();
    GL_CHECK_ERROR();
    glFinish(); //TODO: this causes an explicit flush to ensure that we've actually sync'd with the gpu before deleting the memory. Remove/refactor this code to not throw away meshes anymore.
    delete mesh;
    delete thingToRender;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawVBO_PCT(unsigned int vboID, int numVerts, DrawMode drawMode /*= TRIANGLE_STRIP*/, Texture* texture /*= nullptr*/)
{
    if (!texture)
    {
        texture = m_defaultTexture;
    }
    BindTexture(*texture);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, pos));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, color));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCT), (const GLvoid*)offsetof(Vertex_PCT, texCoords));

    glDrawArrays(GetDrawMode(drawMode), 0, numVerts);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    UnbindTexture();
}

void OGLRenderer::DrawVBO_PCUTB(unsigned int vboID, int numVerts, DrawMode drawMode /*= TRIANGLE_STRIP*/, Texture* texture /*= nullptr*/)
{
    if (!texture)
    {
        texture = m_defaultTexture;
    }
    BindTexture(*texture);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, pos));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, color));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, texCoords));
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, tangent));
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex_PCUTB), (const GLvoid*)offsetof(Vertex_PCUTB, bitangent));

    glDrawArrays(GetDrawMode(drawMode), 0, numVerts);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    UnbindTexture();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawText2D
    ( const Vector2& startBottomLeft
    , const std::string& asciiText
    , float cellWidth
    , float cellHeight
    , const RGBA& tint /*= RGBA::WHITE*/
    , bool drawShadow /*= false*/
    , const BitmapFont* font /*= nullptr*/)
{
    const float SHADOW_WIDTH_OFFSET = cellWidth / 10.0f;
    const float SHADOW_HEIGHT_OFFSET = cellHeight / -10.0f;
    const Vector2 SHADOW_OFFSET = Vector2(SHADOW_WIDTH_OFFSET, SHADOW_HEIGHT_OFFSET);

    if (asciiText.empty())
    {
        return;
    }
    MeshBuilder builder;
    builder.Begin();
    if (font == nullptr)
    {
        font = m_defaultFont;
    }
    int stringLength = asciiText.size();
    Vector2 currentPosition = startBottomLeft;
    for (int i = 0; i < stringLength; i++)
    {
        unsigned char currentCharacter = asciiText[i];
        Vector2 topRight = currentPosition + Vector2(cellWidth, cellHeight);
        AABB2 quadBounds = AABB2(currentPosition, topRight);
        AABB2 glyphBounds =	font->GetTexCoordsForGlyph(currentCharacter);
        if (drawShadow)
        {
            AABB2 shadowBounds = AABB2(currentPosition + SHADOW_OFFSET, topRight + SHADOW_OFFSET);
            RGBA shadowColor = RGBA::BLACK;
            shadowColor.alpha = tint.alpha;
            builder.AddTexturedAABB(shadowBounds, glyphBounds.mins, glyphBounds.maxs, shadowColor);
        }
        builder.AddTexturedAABB(quadBounds, glyphBounds.mins, glyphBounds.maxs, tint);
        currentPosition.x += cellWidth;
    }
    builder.End();

    Mesh mesh;
    builder.CopyToMesh(&mesh, &Vertex_PCUTB::Copy, sizeof(Vertex_PCUTB), &Vertex_PCUTB::BindMeshToVAO);
    mesh.m_drawMode = DrawMode::TRIANGLES;
    MeshRenderer thingToRender(&mesh, font->GetMaterial());
    m_defaultMaterial->SetMatrices(Matrix4x4::IDENTITY, m_viewStack.GetTop(), m_projStack.GetTop());
    GL_CHECK_ERROR();
    thingToRender.Render();
    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawText2D(const Vector2& position, const std::string& asciiText, float scale, const RGBA& tint /*= RGBA::WHITE*/, bool drawShadow /*= false*/, const BitmapFont* font /*= nullptr*/, const Vector2& right /*= Vector3::UNIT_X*/, const Vector2& up /*= Vector3::UNIT_Z*/)
{
    //To be used when I expand this method to 3D text
    UNUSED(up);
    UNUSED(right);
    MeshBuilder builder;
    builder.Begin();
    builder.AddText2D(position, asciiText, scale, tint, drawShadow, font);
    builder.End();

    if (builder.m_vertices.size() == 0)
    {
        return;
    }

    Mesh* mesh = new Mesh();
    builder.CopyToMesh(mesh, &Vertex_PCT::Copy, sizeof(Vertex_PCT), &Vertex_PCT::BindMeshToVAO);
    mesh->m_drawMode = DrawMode::TRIANGLES;
    Material* fontMaterial = font->GetMaterial();
    MeshRenderer* thingToRender = new MeshRenderer(mesh, fontMaterial);
    fontMaterial->SetMatrices(Matrix4x4::IDENTITY, m_viewStack.GetTop(), m_projStack.GetTop());
    GL_CHECK_ERROR();
    thingToRender->Render();
    GL_CHECK_ERROR();
    glFinish(); //TODO: this causes an explicit flush to ensure that we've actually sync'd with the gpu before deleting the memory. Remove/refactor this code to not throw away meshes anymore.
    delete mesh;
    delete thingToRender;
}

//-----------------------------------------------------------------------------------
unsigned char OGLRenderer::GetDrawMode(DrawMode mode) const
{
    switch (mode)
    {
    case DrawMode::QUADS:
        return GL_QUADS;
    case DrawMode::QUAD_STRIP:
        return GL_QUAD_STRIP;
    case DrawMode::POINTS:
        return GL_POINTS;
    case DrawMode::LINES:
        return GL_LINES;
    case DrawMode::LINE_LOOP:
        return GL_LINE_LOOP;
    case DrawMode::POLYGON:
        return GL_POLYGON;
    case DrawMode::TRIANGLES:
        return GL_TRIANGLES;
    case DrawMode::TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;
    default:
        return GL_POINTS;
    }
}

//-----------------------------------------------------------------------------------
GLuint OGLRenderer::GenerateVAOHandle()
{
    GLuint vaoID;
    glGenVertexArrays(1, &vaoID);
    ASSERT_OR_DIE(vaoID != NULL, "VAO was null");
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackVAOAllocation();
    #endif
    return vaoID;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DeleteVAOHandle(GLuint vaoID)
{
    glDeleteVertexArrays(1, &vaoID);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackVAOFree();
    #endif
}

//-----------------------------------------------------------------------------------
void OGLRenderer::ClearDepth(float depthValue)
{
    glClearDepth(depthValue);
    glClear(GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::UseShaderProgram(GLuint shaderProgramID)
{
    if (shaderProgramID == m_currentShaderProgramId)
    {
        return;
    }
    glUseProgram(shaderProgramID);
    m_currentShaderProgramId = shaderProgramID;
}

//-----------------------------------------------------------------------------------
GLuint OGLRenderer::CreateRenderBuffer(size_t size, void* data /*= nullptr*/)
{
    GLuint uboid;
    glGenBuffers(1, &uboid);
    //TODO: This could be more reusable, pass in the usage and target to make new kinds of disgusting buffers <3
    glBindBuffer(GL_UNIFORM_BUFFER, uboid);
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    return uboid;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BindUniform(unsigned int bindPoint, UniformBuffer& buffer)
{
    buffer.CopyToGPU();
    glBindBufferBase(GL_UNIFORM_BLOCK, bindPoint, buffer.m_bufferHandle);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::UnbindIbo()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

//-----------------------------------------------------------------------------------
GLuint OGLRenderer::RenderBufferCreate(void* data, size_t count, size_t elementSize, GLenum usage/* = GL_STATIC_DRAW*/)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, count * elementSize, data, usage);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);

    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferAllocation();
    #endif

    return buffer;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::RenderBufferDestroy(GLuint buffer)
{
    glDeleteBuffers(1, &buffer);

    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackRenderBufferFree();
    #endif
}

//-----------------------------------------------------------------------------------
int OGLRenderer::CreateSampler(GLenum min_filter, //fragment counts for more than one texel, how does it shrink?
    GLenum magFilter, //more texels than fragments, how does it stretch?
    GLenum uWrap, //If u is < 0 or > 1, how does it behave?
    GLenum vWrap) //Same, but for v
{
    GLuint id;
    glGenSamplers(1, &id);

    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, min_filter);
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, magFilter);
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, uWrap); //For some reason, OpenGL refers to UV's as ST's
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, vWrap);

    return id;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DeleteSampler(GLuint id)
{
    glDeleteSamplers(1, &id);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::GLCheckError(const char* file, size_t line)
{
#ifdef CHECK_GL_ERRORS
    GLenum error = glGetError();
    if (error != 0)
    {
        const char* errorText;
        switch (error)
        {
        case GL_INVALID_OPERATION:
            errorText = "INVALID_OPERATION"; 
            break;
        case GL_INVALID_ENUM:
            errorText = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorText = "INVALID_VALUE";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorText = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorText = "OUT_OF_MEMORY";
            break;
        default:
            errorText = "Invalid Enum Value, please debug.";
            break;
        }
        ERROR_RECOVERABLE(Stringf("OpenGL Error: %s\n File Name: %s at %i : \n", errorText, file, line));
    }
#endif
}

//-----------------------------------------------------------------------------------
void OGLRenderer::EnableFaceCulling(bool enabled)
{
    if (m_faceCullingEnabled == enabled)
    {
        return;
    }
    enabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    m_faceCullingEnabled = enabled;
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawPolygonOutline(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color)
{
    Vertex_PCT* vertexes = new Vertex_PCT[numSides];
    const float radiansTotal = 2.0f * MathUtils::PI;
    const float radiansPerSide = radiansTotal / numSides;
    int index = 0;

    for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
    {
        float adjustedRadians = radians + radianOffset;
        float x = center.x + (radius * cos(adjustedRadians));
        float y = center.y + (radius * sin(adjustedRadians));

        vertexes[index].color = color;
        vertexes[index].pos = Vector2(x, y);
    }
    DrawVertexArray(vertexes, numSides, DrawMode::LINE_LOOP);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawPolygon(const Vector2& center, float radius, int numSides, float radianOffset, const RGBA& color)
{
    Vertex_PCT* vertexes = new Vertex_PCT[numSides];
    const float radiansTotal = 2.0f * MathUtils::PI;
    const float radiansPerSide = radiansTotal / numSides;
    int index = 0;

    for (float radians = 0.0f; radians < radiansTotal; radians += radiansPerSide)
    {
        float adjustedRadians = radians + radianOffset;
        float x = center.x + (radius * cos(adjustedRadians));
        float y = center.y + (radius * sin(adjustedRadians));

        vertexes[index].color = color;
        vertexes[index].pos = Vector2(x, y);
    }
    DrawVertexArray(vertexes, numSides, DrawMode::POLYGON);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawAABB(const AABB2& bounds, const RGBA& color)
{
    const int NUM_VERTS = 4;
    Vertex_PCT vertexes[NUM_VERTS];
    vertexes[0].color = color;
    vertexes[1].color = color;
    vertexes[2].color = color;
    vertexes[3].color = color;
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::TRIANGLE_STRIP);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawAABB(const AABB3& bounds, const RGBA& color)
{
    const int NUM_VERTS = 24;
    Vertex_PCT vertexes[NUM_VERTS];
    for (int i = 0; i < NUM_VERTS; i++)
    {
        vertexes[i].color = color;
    }
    //Bottom
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

    //Top
    vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[5].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[6].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);

    //North
    vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);

    //South
    vertexes[12].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[13].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[14].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[15].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);

    //East
    vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[17].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[18].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[19].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);

    //West
    vertexes[20].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[21].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[22].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[23].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);

    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::TRIANGLE_STRIP);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawAABBBoundingBox(const AABB3& bounds, const RGBA& color)
{
    const int NUM_VERTS = 20;
    Vertex_PCT vertexes[NUM_VERTS];
    for (int i = 0; i < NUM_VERTS; i++)
    {
        vertexes[i].color = color;
    }
    //Bottom
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);

    //West
    vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[5].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[6].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);

    //North
    vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);

    //East
    vertexes[12].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[13].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[14].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[15].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);

    //South
    vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[17].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[18].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[19].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);

    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::LINE_LOOP);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawTexturedAABB3(const AABB3& bounds, const RGBA& color, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture)
{
    if (texture == nullptr)
    {
        texture = m_defaultTexture;
    }
    const int NUM_VERTS = 20;
    Vertex_PCT vertexes[NUM_VERTS];
    for (int i = 0; i < NUM_VERTS; i++)
    {
        vertexes[i].color = color;
    }
    //Bottom
    vertexes[0].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[1].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[2].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[3].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[0].texCoords = texCoordMins;
    vertexes[1].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[2].texCoords = texCoordMaxs;
    vertexes[3].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //West
    vertexes[4].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[5].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[6].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[7].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[4].texCoords = texCoordMins;
    vertexes[5].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[6].texCoords = texCoordMaxs;
    vertexes[7].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //North
    vertexes[8].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
    vertexes[9].pos = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[10].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[11].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[8].texCoords = texCoordMins;
    vertexes[9].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[10].texCoords = texCoordMaxs;
    vertexes[11].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //East
    vertexes[12].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[13].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
    vertexes[14].pos = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
    vertexes[15].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[12].texCoords = texCoordMins;
    vertexes[13].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[14].texCoords = texCoordMaxs;
    vertexes[15].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    //South
    vertexes[16].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
    vertexes[17].pos = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
    vertexes[18].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
    vertexes[19].pos = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
    vertexes[16].texCoords = texCoordMins;
    vertexes[17].texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[18].texCoords = texCoordMaxs;
    vertexes[19].texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);

    DrawVertexArray(vertexes, NUM_VERTS, DrawMode::TRIANGLE_STRIP);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawTexturedAABB(const AABB2& bounds, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture, const RGBA& color)
{
    Vertex_PCT vertexes[4];
    Vertex_PCT vertex;
    vertex.color = color;
    vertex.pos = Vector3(bounds.mins.x, bounds.mins.y, 0.0f);
    vertex.texCoords = texCoordMins;
    vertexes[0] = vertex;
    vertex.pos = Vector3(bounds.mins.x, bounds.maxs.y, 0.0f);
    vertex.texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);
    vertexes[1] = vertex;
    vertex.pos = Vector3(bounds.maxs.x, bounds.mins.y, 0.0f);
    vertex.texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[2] = vertex;
    vertex.pos = Vector3(bounds.maxs.x, bounds.maxs.y, 0.0f);
    vertex.texCoords = texCoordMaxs;
    vertexes[3] = vertex;
    m_defaultMaterial->SetDiffuseTexture(texture);
    DrawVertexArray(vertexes, 4, DrawMode::TRIANGLE_STRIP);
    m_defaultMaterial->SetDiffuseTexture(m_defaultTexture);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::DrawTexturedFace(const Face& face, const Vector2& texCoordMins, const Vector2& texCoordMaxs, Texture* texture, const RGBA& color)
{
    UNUSED(texture);
    Vertex_PCT vertexes[4];
    Vertex_PCT vertex;
    vertex.color = color;
    vertex.pos = face.verts[0];
    vertex.texCoords = texCoordMins;
    vertexes[0] = vertex;
    vertex.pos = face.verts[1];
    vertex.texCoords = Vector2(texCoordMaxs.x, texCoordMins.y);
    vertexes[1] = vertex;
    vertex.pos = face.verts[2];
    vertex.texCoords = texCoordMaxs;
    vertexes[2] = vertex;
    vertex.pos = face.verts[3];
    vertex.texCoords = Vector2(texCoordMins.x, texCoordMaxs.y);
    vertexes[3] = vertex;
    DrawVertexArray(vertexes, 4, DrawMode::TRIANGLE_STRIP);
}

//-----------------------------------------------------------------------------------
void OGLRenderer::SetRenderTargets(size_t colorCount, Texture** inColorTargets, Texture* depthStencilTarget)
{
    static const int MAX_NUM_RENDER_TARGETS = 8;
    ASSERT_OR_DIE(colorCount > 0, "Color count wasn't > 0");
    Texture* color0 = inColorTargets[0];
    uint32_t width = color0->m_texelSize.x;
    uint32_t height = color0->m_texelSize.y;

    for (uint32_t i = 0; i < colorCount; ++i)
    {
        Texture* color = inColorTargets[i];
        ASSERT_OR_DIE(((uint32_t)color->m_texelSize.x == width) && ((uint32_t)color->m_texelSize.y == height), "Color target didn't match the height and width of the first target");
    }

    if (nullptr != depthStencilTarget)
    {
        ASSERT_OR_DIE(((uint32_t)depthStencilTarget->m_texelSize.x == width) && ((uint32_t)depthStencilTarget->m_texelSize.y == height), "Depth Stencil Target didn't match the height and width of the first target");
    }

    if (m_fboHandle == NULL)
    {
        glGenFramebuffers(1, &m_fboHandle);
        ASSERT_OR_DIE(m_fboHandle != NULL, "Failed to grab fbo handle");
    }
    
    //OpenGL initialization stuff
    //If you bound a framebuffer to your Renderer, be careful you didn't unbind just now...
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);
    Renderer::instance->SetViewport(0, 0, width, height);

    //Bind our color targets to our FBO
    for (uint32_t i = 0; i < colorCount; ++i)
    {
        Texture* tex = inColorTargets[i];
        glFramebufferTexture(GL_FRAMEBUFFER, //What we're attaching
            GL_COLOR_ATTACHMENT0 + i, //Where we're attaching
            tex->m_openglTextureID, //OpenGL id
            0); //Level - probably mipmap level
        GL_CHECK_ERROR();
    }
    for (uint32_t i = colorCount; i < MAX_NUM_RENDER_TARGETS; i++)
    {
        Texture* tex = inColorTargets[i];
        glFramebufferTexture(GL_FRAMEBUFFER, //What we're attaching
            GL_COLOR_ATTACHMENT0 + i, //Where we're attaching
            NULL, //OpenGL id
            0); //Level - probably mipmap level
        GL_CHECK_ERROR();
    }

    //Bind depth stencil if you have it.
    if (nullptr != depthStencilTarget)
    {
        glFramebufferTexture(GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            depthStencilTarget->m_openglTextureID,
            0);
        GL_CHECK_ERROR();
    }

    //Make sure everything was bound correctly, no errors!
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, NULL);
        glDeleteFramebuffers(1, &m_fboHandle);
        ERROR_RECOVERABLE("Error occured while binding framebuffer");
    }
}

//-----------------------------------------------------------------------------------
void OGLRenderer::BindFramebuffer(Framebuffer* fbo)
{
    if (m_fbo == fbo)
    {
        return;
    }

    m_fbo = fbo;
    if (fbo == nullptr)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, NULL);

        Renderer::instance->SetViewport(0, 0, m_windowSize.x, m_windowSize.y);

    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->m_fboHandle);
        Renderer::instance->SetViewport(0, 0, fbo->m_pixelWidth, fbo->m_pixelHeight);

        GLenum renderTargets[32];
        memset(renderTargets, 0, sizeof(renderTargets));
        for (uint32_t i = 0; i < fbo->m_colorCount; ++i)
        {
            renderTargets[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        glDrawBuffers(fbo->m_colorCount, //How many
            renderTargets); //What do they render to?

    }
}

//-----------------------------------------------------------------------------------
void OGLRenderer::FrameBufferCopyToBack(Framebuffer* fbo, uint32_t drawingWidth, uint32_t drawingHeight, uint32_t bottomLeftX /*= 0*/, uint32_t bottomLeftY /*= 0*/, int colorTargetNumber /*= NULL*/)
{
    UNUSED(colorTargetNumber);
    if (fbo == nullptr)
    {
        return;
    }

    GLuint fboHandle = fbo->m_fboHandle;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboHandle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, NULL);

    uint32_t readWidth = fbo->m_pixelWidth;
    uint32_t readHeight = fbo->m_pixelHeight;

    uint32_t topRightX = bottomLeftX + drawingWidth;
    uint32_t topRightY = bottomLeftY + drawingHeight;

    glBlitFramebuffer(0, 0, //Lower left corner pixel of the read buffer
        readWidth, readHeight, //Top right corner pixel
        bottomLeftX, bottomLeftY, //lower left corner pixel
        topRightX, topRightY, //top right pixel of read buffer
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);

    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
void OGLRenderer::RenderFullScreenEffect(Material* material)
{
    m_fboFullScreenEffectQuad->m_material = material;
    m_fboFullScreenEffectQuad->Render();
}
