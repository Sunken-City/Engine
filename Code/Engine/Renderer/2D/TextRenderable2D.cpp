#include "Engine/Renderer/2D/TextRenderable2D.hpp"
#include "SpriteGameRenderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"

//-----------------------------------------------------------------------------------
TextRenderable2D::TextRenderable2D(const std::string& text, const Transform2D& position, int orderingLayer, bool isEnabled)
    : Renderable2D(orderingLayer, isEnabled)
    , m_text(text)
    , m_font(BitmapFont::CreateOrGetFont("Runescape"))
    , m_transform(position)
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
    static const float TEXT_SANITY_CONSTANT = 0.05f;
    renderer.SetMaterial(m_font->GetMaterial());
    renderer.SetDiffuseTexture(m_font->GetTexture());

    Matrix4x4 scale = Matrix4x4::IDENTITY;
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    Matrix4x4 translation = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeScale(&scale, Vector3(m_transform.GetWorldScale(), 0.0f));
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(m_transform.GetWorldRotationDegrees()));
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(m_transform.GetWorldPosition(), 0.0f));
    renderer.SetModelMatrix(scale * rotation * translation);

    AABB2 bounds = m_font->CalcTextBounds(m_text, TEXT_SANITY_CONSTANT * m_fontSize);
    Vector2 size = Vector2(bounds.GetWidth(), bounds.GetHeight());
    renderer.m_builder.AddText2D(size * -0.5f, m_text, TEXT_SANITY_CONSTANT * m_fontSize, m_color, true, m_font);
    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);

#pragma todo("This should be unneccessary once we have batching done properly")
    renderer.FlushAndRender();
}

//-----------------------------------------------------------------------------------
AABB2 TextRenderable2D::GetBounds()
{
    return AABB2();
}
