#include "Engine/Renderer/2D/BarGraphRenderable2D.hpp"
#include "Engine/Renderer/Material.hpp"
#include "../BufferedMeshRenderer.hpp"

//-----------------------------------------------------------------------------------
BarGraphRenderable2D::BarGraphRenderable2D(const AABB2& bounds, RGBA filledColor, RGBA unfilledColor /*= RGBA::GRAY*/, int orderingLayer /*= 0*/, bool isEnabled /*= true*/)
    : Renderable2D(orderingLayer, isEnabled)
    , m_bounds(bounds)
    , m_fillColor(filledColor)
    , m_unfilledColor(unfilledColor)
    , m_percentageFilled(0.5f)
    , m_isVertical(bounds.GetWidth() < bounds.GetHeight())
    , m_material(Renderer::instance->m_defaultMaterial)
{

}

//-----------------------------------------------------------------------------------
BarGraphRenderable2D::~BarGraphRenderable2D()
{

}

//-----------------------------------------------------------------------------------
void BarGraphRenderable2D::Update(float deltaSeconds)
{
    m_animatedPercentageFilled = MathUtils::Lerp(0.1f, m_animatedPercentageFilled, m_percentageFilled);
}

//-----------------------------------------------------------------------------------
AABB2 BarGraphRenderable2D::GetBounds()
{
    return m_bounds;
}

//-----------------------------------------------------------------------------------
void BarGraphRenderable2D::Render(BufferedMeshRenderer& renderer)
{
    m_material->SetFloatUniform("gPercentageFilled", m_animatedPercentageFilled);
    renderer.SetMaterial(m_material);

    unsigned int indices[6] = { 1, 2, 0, 1, 3, 2 };
    Vertex_Sprite verts[4];
    Vector2 uvMins = Vector2::ZERO;
    Vector2 uvMaxs = Vector2::ONE;
    Matrix4x4 scale = Matrix4x4::IDENTITY;
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    Matrix4x4 translation = Matrix4x4::IDENTITY;
    AABB2 filledBounds = m_bounds;
    AABB2 unfilledBounds = m_bounds;

    //Scale the bounding box
    Matrix4x4::MatrixMakeScale(&scale, Vector3(m_transform.GetWorldScale(), 0.0f));

    //Rotate the bounding box
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(m_transform.GetWorldRotationDegrees()));

    //Translate the bounding box
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(m_transform.GetWorldPosition(), 0.0f));

    if (m_isVertical)
    {
        Vector2 filledMidpoint = Vector2(m_bounds.maxs.x, MathUtils::Lerp(Clamp01(m_animatedPercentageFilled), m_bounds.mins.y, m_bounds.maxs.y));
        Vector2 unfilledMidpoint = filledMidpoint;
        unfilledMidpoint.x = m_bounds.mins.x;
        filledBounds = AABB2(m_bounds.mins, filledMidpoint);
        unfilledBounds = AABB2(unfilledMidpoint, m_bounds.maxs);
    }
    else
    {
        Vector2 filledMidpoint = Vector2(MathUtils::Lerp(Clamp01(m_animatedPercentageFilled), m_bounds.mins.x, m_bounds.maxs.x), m_bounds.maxs.y);
        Vector2 unfilledMidpoint = filledMidpoint;
        unfilledMidpoint.y = m_bounds.mins.y;
        filledBounds = AABB2(m_bounds.mins, filledMidpoint);
        unfilledBounds = AABB2(unfilledMidpoint, m_bounds.maxs);
    }


    //Apply our transformations
    renderer.SetModelMatrix(scale * rotation * translation);
    renderer.m_builder.AddTexturedAABB(filledBounds, uvMins, uvMaxs, m_fillColor);
    renderer.m_builder.AddTexturedAABB(unfilledBounds, uvMins, uvMaxs, m_unfilledColor);
    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);

#pragma todo("This should be unneccessary once we have batching done properly")
    renderer.FlushAndRender();
}

//-----------------------------------------------------------------------------------
void BarGraphRenderable2D::SetPercentageFilled(float percentageFilledValue)
{
    m_animatedPercentageFilled = m_percentageFilled;
    m_percentageFilled = percentageFilledValue;
}

