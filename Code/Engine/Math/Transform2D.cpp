#include "Engine/Math/Transform2D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------
Transform2D::Transform2D(const Vector2& pos, float rotDegrees, const Vector2& scaleVal, Transform2D* parent)
    : m_position(pos)
    , m_rotationDegrees(rotDegrees)
    , m_scale(scaleVal)
{
    if (parent)
    {
        SetParent(parent);
    }
}

//-----------------------------------------------------------------------------------
Transform2D::Transform2D(const Transform2D& other)
    : m_position(other.GetWorldPosition())
    , m_rotationDegrees(other.GetWorldRotationDegrees())
    , m_scale(other.GetWorldScale())
{

}

//-----------------------------------------------------------------------------------
Transform2D::~Transform2D()
{
    if (m_parent)
    {
        m_parent->RemoveChild(this);
    }

    for (Transform2D* child : m_children)
    {
        child->RemoveParent();
    }
}

//-----------------------------------------------------------------------------------
void Transform2D::AddChild(Transform2D* child)
{
    m_children.push_back(child);
    child->m_parent = this;
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
            foundChild->RemoveParent();
            return;
        }
    }
    ERROR_RECOVERABLE("Didn't find a child transform to remove");
}

//-----------------------------------------------------------------------------------
void Transform2D::RemoveParent()
{
    m_parent = nullptr;
}

//-----------------------------------------------------------------------------------
Transform2D& Transform2D::operator=(const Transform2D& other)
{
    m_position = other.GetWorldPosition();
    m_rotationDegrees = other.GetWorldRotationDegrees();
    m_scale = other.GetWorldScale();
    return *this;
}

//-----------------------------------------------------------------------------------
void Transform2D::SetParent(Transform2D* parent)
{
    parent->AddChild(this);
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetWorldPosition() const
{
    if (m_parent && m_applyParentTranslation)
    {
        return m_position + m_parent->GetWorldPosition();
    }
    else
    {
        return m_position;
    }
}

//-----------------------------------------------------------------------------------
float Transform2D::GetWorldRotationDegrees() const
{
    if (m_parent && m_applyParentRotation)
    {
        return m_rotationDegrees + m_parent->GetWorldRotationDegrees();
    }
    else
    {
        return m_rotationDegrees;
    }
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetWorldScale() const
{
    if (m_parent && m_applyParentScale)
    {
        return m_scale * m_parent->GetWorldScale();
    }
    else
    {
        return m_scale;
    }
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetLocalPosition() const
{
    return m_position;
}

//-----------------------------------------------------------------------------------
float Transform2D::GetLocalRotationDegrees() const
{
    return m_rotationDegrees;
}

//-----------------------------------------------------------------------------------
Vector2 Transform2D::GetLocalScale() const
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

