#include "Engine/Renderer/BufferedMeshRenderer.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Material.hpp"

//-----------------------------------------------------------------------------------
BufferedMeshRenderer::BufferedMeshRenderer()
    : m_renderer(&m_mesh, Renderer::instance->m_defaultMaterial)
{

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
    m_builder.CopyToMesh(&m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
    if (m_mesh.m_numVerts == 0)
    {
        return;
    }
    m_renderer.Render();
    m_mesh.CleanUpRenderObjects();
}

//-----------------------------------------------------------------------------------
void BufferedMeshRenderer::SetModelMatrix(const Matrix4x4& model)
{
    m_renderer.SetModelMatrix(model);
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

