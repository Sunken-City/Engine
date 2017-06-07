#include "Engine/Renderer/2D/BarGraphRenderable2D.hpp"
#include "Engine/Renderer/Material.hpp"
#include "../BufferedMeshRenderer.hpp"
#include "../../Core/ProfilingUtils.h"

//-----------------------------------------------------------------------------------
BarGraphRenderable2D::BarGraphRenderable2D(const AABB2& bounds, RGBA filledColor, RGBA unfilledColor /*= RGBA::GRAY*/, int orderingLayer /*= 0*/, bool isEnabled /*= true*/)
    : Renderable2D(orderingLayer, isEnabled)
    , m_bounds(bounds)
    , m_fillColor(filledColor)
    , m_unfilledColor(unfilledColor)
    , m_percentageFilled(0.5f)
    , m_material(Renderer::instance->m_defaultMaterial)
{
    if (!AABB2::IsValid(m_bounds))
    {
        m_isLeftToRight = true;
        Vector2 temp = m_bounds.maxs;
        m_bounds.maxs = m_bounds.mins;
        m_bounds.mins = temp;
        ASSERT_OR_DIE(AABB2::IsValid(m_bounds), "Invalid bounds passed into the bar graph");
    }
    m_isVertical = (m_bounds.GetWidth() < m_bounds.GetHeight());

    m_filledMinsTransform.SetParent(&m_transform);
    m_filledMaxsTransform.SetParent(&m_transform);
}

//-----------------------------------------------------------------------------------
BarGraphRenderable2D::~BarGraphRenderable2D()
{

}

//-----------------------------------------------------------------------------------
void BarGraphRenderable2D::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds);
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
    ProfilingSystem::instance->PushSample("BarGraphRenderable2D");
    static size_t gPercentageFilledUniform = std::hash<std::string>{}("gPercentageFilled");
    m_material->SetFloatUniform(gPercentageFilledUniform, m_animatedPercentageFilled);
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
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, DegreesToRadians(m_transform.GetWorldRotationDegrees()));

    //Translate the bounding box
    Matrix4x4::MatrixMakeTranslation(&translation, Vector3(m_transform.GetWorldPosition(), 0.0f));

    float fillPercentage = m_animatedPercentageFilled;

    if (m_isVertical)
    {
        float blendAmount = Clamp01(fillPercentage);
        if (m_isLeftToRight)
        {
            blendAmount = 1.0f - blendAmount;
        }
        Vector2 filledMidpoint = Vector2(m_bounds.maxs.x, MathUtils::Lerp(Clamp01(fillPercentage), m_bounds.mins.y, m_bounds.maxs.y));
        Vector2 unfilledMidpoint = filledMidpoint;
        unfilledMidpoint.x = m_bounds.mins.x;
        filledBounds = AABB2(m_bounds.mins, filledMidpoint);
        unfilledBounds = AABB2(unfilledMidpoint, m_bounds.maxs);
    }
    else
    {
        float blendAmount = Clamp01(fillPercentage);
        if (m_isLeftToRight)
        {
            blendAmount = 1.0f - blendAmount;
        }
        Vector2 filledMidpoint = Vector2(MathUtils::Lerp(blendAmount, m_bounds.mins.x, m_bounds.maxs.x), m_bounds.maxs.y);
        Vector2 unfilledMidpoint = filledMidpoint;
        unfilledMidpoint.y = m_bounds.mins.y;
        filledBounds = AABB2(m_bounds.mins, filledMidpoint);
        unfilledBounds = AABB2(unfilledMidpoint, m_bounds.maxs);
    }

    //If we're going the opposite way, swap the filled and unfilled bounds.
    if (m_isLeftToRight)
    {
        AABB2 temp = filledBounds;
        filledBounds = unfilledBounds;
        unfilledBounds = temp;
    }

    //Update the transforms on the tips of the filled part.
    Vector2 mins = filledBounds.mins;
    Vector2 maxs = filledBounds.maxs;
    if (m_isVertical)
    {
        mins.x += filledBounds.GetWidth() / 2.0f;
        maxs.x -= filledBounds.GetWidth() / 2.0f;
    }
    else
    {
        mins.y += filledBounds.GetHeight() / 2.0f;
        maxs.y -= filledBounds.GetHeight() / 2.0f;
    }
    m_filledMinsTransform.SetPosition(mins);
    m_filledMaxsTransform.SetPosition(maxs);

    //Apply our transformations
    renderer.SetModelMatrix(scale * rotation * translation);
    renderer.m_builder.AddTexturedAABB(filledBounds, uvMins, uvMaxs, m_fillColor);
    renderer.m_builder.AddTexturedAABB(unfilledBounds, uvMins, uvMaxs, m_unfilledColor);
    renderer.m_builder.CopyToMesh(&renderer.m_mesh, &Vertex_Sprite::Copy, sizeof(Vertex_Sprite), &Vertex_Sprite::BindMeshToVAO);

#pragma todo("This should be unneccessary once we have batching done properly")
    renderer.FlushAndRender();
    ProfilingSystem::instance->PopSample("BarGraphRenderable2D");
}

//-----------------------------------------------------------------------------------
void BarGraphRenderable2D::SetPercentageFilled(float percentageFilledValue)
{
    m_animatedPercentageFilled = m_percentageFilled;
    m_percentageFilled = percentageFilledValue;
}

