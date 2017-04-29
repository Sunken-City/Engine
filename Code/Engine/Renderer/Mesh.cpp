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
    , m_vbo(NULL)
    , m_ibo(NULL)
    , m_numIndices(0)
    , m_numVerts(0)
{

}

//-----------------------------------------------------------------------------------
void Mesh::MarkMeshEmpty()
{
    m_numVerts = 0;
    m_numIndices = 0;
}


//-----------------------------------------------------------------------------------
void Mesh::CleanUpRenderObjects()
{
    if (m_vbo != NULL)
    {
        Renderer::instance->DeleteBuffers(m_vbo);
        m_numVerts = 0;
        m_vbo = NULL;
        m_vboBufferSize = 0;
    }
    if (m_ibo != NULL)
    {
        Renderer::instance->RenderBufferDestroy(m_ibo);
        m_numIndices = 0;
        m_ibo = NULL;
        m_iboBufferSize = 0;
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
void Mesh::Update(void* vertexData, unsigned int numVertices, unsigned int sizeofVertex, void* indexData, unsigned int numIndices, BindMeshToVAOForVertex* BindMeshFunction)
{
    m_numVerts = numVertices;
    m_numIndices = numIndices;
    if (BindMeshFunction != m_vertexBindFunctionPointer)
    {
        m_vertexBindFunctionPointer = BindMeshFunction;
        m_isDirty = true;
    }
    unsigned int requiredVBOBufferSize = (numVertices * sizeofVertex);
    unsigned int requiredIBOBufferSize = (numIndices * sizeof(unsigned int));

    if (m_vbo == NULL)
    {
        m_vbo = Renderer::instance->GenerateBufferID(); //Check to make sure this is a deleted VBO first.
        m_isDirty = true;
        GL_CHECK_ERROR();
    }
    if (m_dynamicDraw == false || m_vboBufferSize < requiredVBOBufferSize)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        GL_CHECK_ERROR();
        glBufferData(GL_ARRAY_BUFFER, requiredVBOBufferSize, vertexData, m_dynamicDraw ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        GL_CHECK_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, NULL);
        m_vboBufferSize = requiredVBOBufferSize;
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        GL_CHECK_ERROR();
        glBufferSubData(GL_ARRAY_BUFFER, 0, requiredVBOBufferSize, vertexData);
        glBindBuffer(GL_ARRAY_BUFFER, NULL);
    }

    if (m_ibo == NULL)
    {
        m_ibo = Renderer::instance->GenerateBufferID(); //Check to make sure this is a deleted IBO first.
        m_isDirty = true;
        GL_CHECK_ERROR();
    }
    if (m_dynamicDraw == false || m_iboBufferSize < requiredIBOBufferSize)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
        GL_CHECK_ERROR();
        glBufferData(GL_ARRAY_BUFFER, requiredIBOBufferSize, indexData, m_dynamicDraw ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        GL_CHECK_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, NULL);
        m_iboBufferSize = requiredIBOBufferSize;
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
        GL_CHECK_ERROR();
        glBufferSubData(GL_ARRAY_BUFFER, 0, requiredIBOBufferSize, indexData);
        GL_CHECK_ERROR();
        glBindBuffer(GL_ARRAY_BUFFER, NULL);
    }
    GL_CHECK_ERROR();
}

//-----------------------------------------------------------------------------------
void Mesh::BindToVAO(GLuint vaoID, ShaderProgram* shaderProgram)
{
    if (shaderProgram != m_lastUsedShaderProgram)
    {
        m_lastUsedShaderProgram = shaderProgram;
        m_isDirty = true;
    }
    if (m_isDirty)
    {
        ProfilingSystem::instance->PushSample("BindToVAO");
        m_vertexBindFunctionPointer(vaoID, m_vbo, m_ibo, shaderProgram);
        m_isDirty = false;
        ProfilingSystem::instance->PopSample("BindToVAO");
    }
}


