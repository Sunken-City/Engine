#include "Engine/UI/Widgets/ButtonWidget.hpp"
#include "Engine/Renderer/AABB2.hpp"
#include "Engine/UI/Widgets/LabelWidget.hpp"
#include "../../Renderer/Renderer.hpp"
#include "../../Renderer/MeshBuilder.hpp"
#include "../../Renderer/MeshRenderer.hpp"
#include "../../Renderer/Material.hpp"

//-----------------------------------------------------------------------------------
ButtonWidget::ButtonWidget()
    : WidgetBase()
{
    m_textLabel = new LabelWidget();
    m_children.push_back(m_textLabel);
}

//-----------------------------------------------------------------------------------
ButtonWidget::~ButtonWidget()
{
    delete m_textLabel;
}

//-----------------------------------------------------------------------------------
void ButtonWidget::Update(float deltaSeconds)
{
    WidgetBase::Update(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void ButtonWidget::Render() const
{
//     MeshBuilder builder;
//     builder.AddTexturedAABB(AABB2(Vector2::ZERO, Vector2(1600,900)), Vector2::ZERO, Vector2::ZERO, RGBA::WHITE);
//     Mesh mesh;
//     builder.CopyToMesh(&mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_PCUTB::BindMeshToVAO);
//     mesh.m_drawMode = Renderer::DrawMode::TRIANGLES;
//     Renderer::instance->m_defaultMaterial->SetDiffuseTexture(Renderer::instance->m_defaultTexture);
//     MeshRenderer thingToRender(&mesh, Renderer::instance->m_defaultMaterial);
//     GL_CHECK_ERROR();
//     thingToRender.Render();
    Renderer::instance->DrawAABB(m_bounds, RGBA::CHOCOLATE);

    WidgetBase::Render();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::BuildFromXMLNode(XMLNode& node)
{
    m_textLabel->BuildFromXMLNode(node);

    RecalculateBounds();
}

//-----------------------------------------------------------------------------------
void ButtonWidget::RecalculateBounds()
{
    m_bounds = GetSmallestBoundsAroundChildren();
}