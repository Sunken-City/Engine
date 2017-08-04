#include "Engine/UI/Dimensions.hpp"
#include "Engine/Renderer/AABB2.hpp"

//-----------------------------------------------------------------------------------
AABB2 Dimensions2D::GetAsAABB2()
{
    AABB2 bounds;
    float halfMinWidthX = x.m_minSize;
    float halfMinWidthY = y.m_minSize;

    bounds.mins.x = -halfMinWidthX;
    bounds.maxs.x = halfMinWidthX;
    return bounds;
}

//-----------------------------------------------------------------------------------
void Dimension::SetMinSize(float newMin)
{
    m_minSize = newMin;
    if (m_maxSize < m_minSize)
    {
        m_maxSize = m_minSize;
    }
}

//-----------------------------------------------------------------------------------
void Dimension::SetMaxSize(float newMaximum)
{
    m_maxSize = newMaximum;
    if (m_minSize > m_maxSize)
    {
        m_minSize = m_maxSize;
    }
}
