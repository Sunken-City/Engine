#pragma once
#include <vector>
#include "Engine/Renderer/RGBA.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"

class Vector3Int;
class Vector3;
class ShaderProgram;
class Material;
struct Vertex_PCUTB;


typedef unsigned int GLuint;
typedef void (BindMeshToVAOForVertex)(GLuint vao, GLuint vbo, GLuint ibo, ShaderProgram* program);
class Mesh
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Mesh();
    ~Mesh();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void RenderFromIBO(GLuint vaoID, Material* material) const;
    void MarkMeshEmpty();

    //HELPER FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Update(void* vertexData, unsigned int numVertices, unsigned int sizeofVertex, void* indexData, unsigned int numIndices, BindMeshToVAOForVertex* BindMeshFunction);
    void BindToVAO(GLuint m_vaoID, ShaderProgram* m_shaderProgram);
    void CleanUpRenderObjects();

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    GLuint m_vbo;
    GLuint m_ibo;
    unsigned int m_numVerts;
    unsigned int m_numIndices;
    unsigned int m_vboBufferSize = 0;
    unsigned int m_iboBufferSize = 0;
    bool m_dynamicDraw = false;
    bool m_isDirty = false;
    BindMeshToVAOForVertex* m_vertexBindFunctionPointer = nullptr;
    ShaderProgram* m_lastUsedShaderProgram = nullptr;
    Renderer::DrawMode m_drawMode;

private:
    //Prevents copy by value. We can't copy the GL resources, so we don't want to be able to accidentally copy these.
    Mesh(const Mesh&);
};