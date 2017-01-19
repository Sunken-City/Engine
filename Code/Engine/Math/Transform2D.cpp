#include "Engine/Math/Transform2D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

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
    child->SetParent(this);
}

//-----------------------------------------------------------------------------------
void Transform2D::RemoveChild(Transform2D* child)
{
    if (!child)
    {
        ERROR_RECOVERABLE("Attempted to remove a null child transform");
        return;
    }

    unsigned int numChildren = m_children.size();
    for (unsigned int i = 0; i < numChildren; ++i)
    {
        Transform2D* foundChild = m_children[i];
        if (foundChild == child)
        {
            m_children[i] = m_children[numChildren - 1];
            m_children.pop_back();
            return;
        }
    }
    ERROR_RECOVERABLE("Didn't find a child transform to remove");
}

//-----------------------------------------------------------------------------------
void Transform2D::SetParent(Transform2D* parent)
{
    m_parent = parent;
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetWorldPosition()
{
    if (m_parent)
    {
        return m_position + m_parent->GetWorldPosition();
    }
    else
    {
        return m_position;
    }
}

//-----------------------------------------------------------------------------------
float Transform2D::GetWorldRotationDegrees()
{
    if (m_parent)
    {
        return m_rotationDegrees + m_parent->GetWorldRotationDegrees();
    }
    else
    {
        return m_rotationDegrees;
    }
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetWorldScale()
{
    if (m_parent)
    {
        return m_scale + m_parent->GetWorldScale();
    }
    else
    {
        return m_scale;
    }
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetLocalPosition()
{
    return m_position;
}

//-----------------------------------------------------------------------------------
float Transform2D::GetLocalRotationDegrees()
{
    return m_rotationDegrees;
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetLocalScale()
{
    return m_scale;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetPosition(const Vector2& position)
{
    m_position = position;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetRotationDegrees(float rotationDegrees)
{
    m_rotationDegrees = rotationDegrees;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetScale(const Vector2& scale)
{
    m_scale = scale;
}

