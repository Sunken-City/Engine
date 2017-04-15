#include "Engine/Renderer/2D/TextRenderable2D.hpp"
#include "SpriteGameRenderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "../../Core/ProfilingUtils.h"

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
void TextRenderable2D::Update(float)
{
    static const float TEXT_SANITY_CONSTANT = 0.05f;
    m_bounds = m_font->CalcTextBounds(m_text, TEXT_SANITY_CONSTANT * m_fontSize * m_transform.GetWorldScale().x);
    Vector2 size = Vector2(m_bounds.GetWidth(), m_bounds.GetHeight());
    m_bounds += size * -0.5f;
    m_bounds += m_transform.GetWorldPosition();
}

//-----------------------------------------------------------------------------------
void TextRenderable2D::Render(BufferedMeshRenderer& renderer)
{
    //ProfilingSystem::instance->PushSample("TextRenderable2D");
    static const float TEXT_SANITY_CONSTANT = 0.05f;
    renderer.SetMaterial(m_font->GetMaterial());
    renderer.SetDiffuseTexture(m_font->GetTexture());

    Vector2 worldScale = m_transform.GetWorldScale();
    Matrix4x4 scale = Matrix4x4::IDENTITY;
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    Matrix4x4 translation = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeScale(&scale, Vector3(worldScale, 0.0f));
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(m_transform.GetWorldRotationDegrees()));
    //Matrix4x4::MatrixMakeTranslation(&translation, Vector3(0.0f));
    renderer.SetModelMatrix(scale * rotation * translation);

    float finalScale = TEXT_SANITY_CONSTANT * m_fontSize * worldScale.x;
    //m_bounds = m_font->CalcTextBounds(m_text, finalScale);
    Vector2 size = Vector2(m_bounds.GetWidth() * -0.5f, m_bounds.GetHeight() * -0.5f);

    //ProfilingSystem::instance->PushSample("AddingText");
    renderer.m_builder.AddText2D(size + m_transform.GetWorldPosition(), m_text, finalScale, m_color, true, m_font);
    //ProfilingSystem::instance->PopSample("AddingText");
    //ProfilingSystem::instance->PushSample("CopyToMesh");
    //ProfilingSystem::instance->PopSample("CopyToMesh");

    //m_bounds += size * -0.5f;
    //m_bounds += m_transform.GetWorldPosition();

    //ProfilingSystem::instance->PushSample("Flush&Render");
    //ProfilingSystem::instance->PopSample("Flush&Render");
    //ProfilingSystem::instance->PopSample("TextRenderable2D");
}

//-----------------------------------------------------------------------------------
AABB2 TextRenderable2D::GetBounds()
{
    return m_bounds;
}
