#include "Engine/Renderer/BufferedMeshRenderer.hpp"
#include "Engine/Renderer/Vertex.hpp"

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
    m_renderer.Render();
}

