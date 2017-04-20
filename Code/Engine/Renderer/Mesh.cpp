#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Math/Vector3Int.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "../Core/ProfilingUtils.h"

//-----------------------------------------------------------------------------------
Mesh::Mesh()
    : m_drawMode(Renderer::DrawMode::TRIANGLES)
    , m_vbo(0)
    , m_ibo(0)
    , m_numIndices(0)
    , m_numVerts(0)
{

}

//-----------------------------------------------------------------------------------
void Mesh::CleanUpRenderObjects()
{
    if (m_vbo != 0)
    {
        Renderer::instance->DeleteBuffers(m_vbo);
        m_numVerts = 0;
        m_vbo = 0;
    }
    if (m_ibo != 0)
    {
        Renderer::instance->RenderBufferDestroy(m_ibo);
        m_numIndices = 0;
        m_ibo = 0;
    }
}

//-----------------------------------------------------------------------------------
Mesh::~Mesh()
{
    CleanUpRenderObjects();
}

//-----------------------------------------------------------------------------------
void Mesh::RenderFromIBO(GLuint vaoID, Material* material) const
{
    ProfilingSystem::instance->PushSample("RenderFromIBO");
    glBindVertexArray(vaoID);
    material->SetUpRenderState();
    //Draw with IBO
    glDrawElements(Renderer::instance->GetDrawMode(m_drawMode), m_numIndices, GL_UNSIGNED_INT, (GLvoid*)0);
    //material->CleanUpRenderState();
    glBindVertexArray(NULL);
    ProfilingSystem::instance->PopSample("RenderFromIBO");
}

//Pushes data over to the GPU and creates the buffers. The mesh doesn't store any of the vertexes or indexes, just the buffer locations.
//-----------------------------------------------------------------------------------
void Mesh::Update(void* vertexData, unsigned int numVertices, unsigned int sizeofVertex, void* indexData, unsigned int numIndices, BindMeshToVAOForVertex* BindMeshFunction, bool dynamicDraw)
{
    m_numVerts = numVertices;
    m_numIndices = numIndices;
    m_vertexBindFunctionPointer = BindMeshFunction;
    m_vbo = Renderer::instance->GenerateBufferID(); //Check to make sure this is a deleted VBO first.
    GL_CHECK_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeofVertex * numVertices, vertexData, dynamicDraw ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    GL_CHECK_ERROR();
    m_ibo = Renderer::instance->RenderBufferCreate(indexData, numIndices, sizeof(unsigned int), GL_STATIC_DRAW);
    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
void Mesh::BindToVAO(GLuint vaoID, ShaderProgram* shaderProgram)
{
    m_vertexBindFunctionPointer(vaoID, m_vbo, m_ibo, shaderProgram);
}


