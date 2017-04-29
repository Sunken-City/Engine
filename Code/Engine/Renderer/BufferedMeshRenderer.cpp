#include "Engine/Renderer/BufferedMeshRenderer.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Material.hpp"
#include "../Core/ProfilingUtils.h"

//-----------------------------------------------------------------------------------
BufferedMeshRenderer::BufferedMeshRenderer()
    : m_renderer(&m_mesh, Renderer::instance->m_defaultMaterial)
{
    m_mesh.m_dynamicDraw = true;
}

//-----------------------------------------------------------------------------------
BufferedMeshRenderer::~BufferedMeshRenderer()
{

}

//-----------------------------------------------------------------------------------
void BufferedMeshRenderer::SetMaterial(Material* newMat)
{
    if (newMat != m_renderer.m_material)
    {
        FlushAndRender();
        m_renderer.m_material = newMat;
    }
}

//-----------------------------------------------------------------------------------
void BufferedMeshRenderer::FlushAndRender()
{
    ProfilingSystem::instance->PushSample("FlushAndRender");
    if (m_mesh.m_numVerts == 0)
    {
        if (m_builder.m_vertices.size() != 0)
        {
            m_builder.CopyToMesh(&m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
        }
        else
        {
            ProfilingSystem::instance->PopSample("FlushAndRender");
            return;
        }
    }
    m_renderer.Render();
    m_mesh.MarkMeshEmpty();
    //m_mesh.CleanUpRenderObjects();
#ifdef PROFILING_ENABLED
    ProfilingSystem::instance->m_activeSample->numDrawCalls += 1;
#endif

    ProfilingSystem::instance->PopSample("FlushAndRender");
}

//-----------------------------------------------------------------------------------
void BufferedMeshRenderer::SetModelMatrix(const Matrix4x4& model)
{
    if (m_renderer.m_model != model)
    {
        FlushAndRender();
        m_renderer.SetModelMatrix(model);
    }
}

//-----------------------------------------------------------------------------------
void BufferedMeshRenderer::SetDiffuseTexture(Texture* diffuseTexture)
{
    if (diffuseTexture->m_openglTextureID != m_renderer.m_material->m_diffuseID)
    {
        FlushAndRender(); 
        m_renderer.m_material->SetDiffuseTexture(diffuseTexture);
    }
}

