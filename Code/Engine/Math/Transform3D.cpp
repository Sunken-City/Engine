#include "Engine/Math/Transform3D.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------
Transform3D::Transform3D(const Vector3& pos, float rotDegrees, const Vector3& scaleVal, Transform3D* parent)
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
Transform3D::Transform3D(const Transform3D& other)
    : m_position(other.GetWorldPosition())
    , m_rotationDegrees(other.GetWorldRotationDegrees())
    , m_scale(other.GetWorldScale())
{

}

//-----------------------------------------------------------------------------------
Transform3D::~Transform3D()
{
    if (m_parent)
    {
        m_parent->RemoveChild(this);
    }

    for (Transform3D* child : m_children)
    {
        child->RemoveParent();
    }
}

//-----------------------------------------------------------------------------------
void Transform3D::AddChild(Transform3D* child)
{
    m_children.push_back(child);
    child->m_parent = this;
}

//-----------------------------------------------------------------------------------
void Transform3D::RemoveChild(Transform3D* child)
{
    if (!child)
    {
        ERROR_RECOVERABLE("Attempted to remove a null child transform");
        return;
    }

    unsigned int numChildren = m_children.size();
    for (unsigned int i = 0; i < numChildren; ++i)
    {
        Transform3D* foundChild = m_children[i];
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
void Transform3D::DropChildrenInPlace()
{
    const Vector3 position = GetWorldPosition();
    for (Transform3D* transform : m_children)
    {
        transform->SetPosition(transform->GetLocalPosition() + position);
    }
}

//-----------------------------------------------------------------------------------
void Transform3D::RemoveParent()
{
    m_parent = nullptr;
}

//-----------------------------------------------------------------------------------
Matrix4x4 Transform3D::GetModelMatrix()
{
    Matrix4x4 scale = Matrix4x4::IDENTITY;
    Matrix4x4 rotation = Matrix4x4::IDENTITY;
    Matrix4x4 translation = Matrix4x4::IDENTITY;

    Matrix4x4::MatrixMakeScale(&scale, GetWorldScale());
    Matrix4x4::MatrixMakeRotationAroundZ(&rotation, MathUtils::DegreesToRadians(GetWorldRotationDegrees()));
    Matrix4x4::MatrixMakeTranslation(&translation, GetWorldPosition());

    //Apply our transformations
    return (scale * rotation * translation);
}

//-----------------------------------------------------------------------------------
Transform3D& Transform3D::operator=(const Transform3D& other)
{
    m_position = other.GetWorldPosition();
    m_rotationDegrees = other.GetWorldRotationDegrees();
    m_scale = other.GetWorldScale();
    return *this;
}

//-----------------------------------------------------------------------------------
void Transform3D::SetParent(Transform3D* parent)
{
    parent->AddChild(this);
}

//-----------------------------------------------------------------------------------
Vector3 Transform3D::GetWorldPosition() const
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
float Transform3D::GetWorldRotationDegrees() const
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
Vector3 Transform3D::GetWorldScale() const
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
Vector3 Transform3D::GetLocalPosition() const
{
    return m_position;
}

//-----------------------------------------------------------------------------------
float Transform3D::GetLocalRotationDegrees() const
{
    return m_rotationDegrees;
}

//-----------------------------------------------------------------------------------
Vector3 Transform3D::GetLocalScale() const
{
    return m_scale;
}

//-----------------------------------------------------------------------------------
void Transform3D::SetPosition(const Vector3& position)
{
    m_position = position;
}

//-----------------------------------------------------------------------------------
void Transform3D::SetRotationDegrees(float rotationDegrees)
{
    m_rotationDegrees = rotationDegrees;
}

//-----------------------------------------------------------------------------------
void Transform3D::SetScale(const Vector3& scale)
{
    m_scale = scale;
}

