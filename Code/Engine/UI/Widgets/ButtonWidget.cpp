#include "Engine/UI/Widgets/ButtonWidget.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "../UISystem.hpp"

//-----------------------------------------------------------------------------------
ButtonWidget::ButtonWidget()
    : LabelWidget()
{

}

//-----------------------------------------------------------------------------------
ButtonWidget::~ButtonWidget()
{

}

//-----------------------------------------------------------------------------------
void ButtonWidget::Update(float deltaSeconds)
{
    LabelWidget::Update(deltaSeconds);
    UpdateChildren(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void ButtonWidget::Render() const
{
//     MeshBuilder builder;
//     builder.AddTexturedAABB(AABB2(Vector2::ZERO, Vector2(1600,900)), Vector2::ZERO, Vector2::ZERO, RGBA::WHITE);
//     Mesh mesh;
//     builder.CopyToMesh(&mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);
//     mesh.m_drawMode = Renderer::DrawMode::TRIANGLES;
//     Renderer::instance->m_defaultMaterial->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
//     MeshRenderer thingToRender(&mesh, Renderer::instance->m_defaultMaterial);
//     GL_CHECK_ERROR();
//     thingToRender.Render();

    LabelWidget::Render();
    RenderChildren();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::BuildFromXMLNode(XMLNode& node)
{
    LabelWidget::BuildFromXMLNode(node);

    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::RecalculateBounds()
{
    LabelWidget::RecalculateBounds();
    m_bounds += m_propertiesForAllStates.Get<Vector2>("Offset");
}