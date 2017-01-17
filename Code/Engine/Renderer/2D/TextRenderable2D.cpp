#include "Engine/Renderer/2D/TextRenderable2D.hpp"
#include "SpriteGameRenderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

//-----------------------------------------------------------------------------------
TextRenderable2D::TextRenderable2D(const std::string& text, int orderingLayer, bool isEnabled)
    : Renderable2D(orderingLayer, isEnabled)
    , m_text(text)
    , m_font(BitmapFont::CreateOrGetFont("Runescape"))
{

}

//-----------------------------------------------------------------------------------
TextRenderable2D::~TextRenderable2D()
{

}

//-----------------------------------------------------------------------------------
void TextRenderable2D::Update(float deltaSeconds)
{

}

//-----------------------------------------------------------------------------------
void TextRenderable2D::Render(BufferedMeshRenderer& renderer)
{
    renderer.SetMaterial(m_font->GetMaterial());
    renderer.SetDiffuseTexture(m_font->GetTexture());
    renderer.m_builder.AddText2D(Vector2::ZERO, m_text, 0.005f, RGBA::WHITE, true, m_font);
    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);

#pragma todo("This should be unneccessary once we have batching done properly")
    renderer.FlushAndRender();
}

//-----------------------------------------------------------------------------------
AABB2 TextRenderable2D::GetBounds()
{
    return AABB2();
}
