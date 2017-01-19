#include "Engine/Math/Transform2D.hpp"

//-----------------------------------------------------------------------------------
Transform2D::Transform2D(const Vector2& pos, float rotDegrees, const Vector2& scaleVal, Transform2D* parent)
    : m_position(pos)
    , m_rotationDegrees(rotDegrees)
    , m_scale(scaleVal)
{
    SetParent(parent);
}

//-----------------------------------------------------------------------------------
void Transform2D::AddChild(Transform2D* child)
{
    m_children.push_back(child);
}

//-----------------------------------------------------------------------------------
void Transform2D::SetParent(Transform2D* parent)
{
    m_parent = parent;
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetPosition()
{
    return m_position;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetPosition(const Vector2& position)
{
    m_position = position;
}

//-----------------------------------------------------------------------------------
float Transform2D::GetRotationDegrees()
{
    return m_rotationDegrees;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetRotationDegrees(float rotationDegrees)
{
    m_rotationDegrees = rotationDegrees;
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetScale()
{
    return m_scale;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetScale(const Vector2& scale)
{
    m_scale = scale;
}

