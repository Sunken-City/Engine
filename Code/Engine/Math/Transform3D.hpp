#pragma once

#include <vector>
#include "Engine/Math/Vector3.hpp"

//-----------------------------------------------------------------------------------
class Transform3D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Transform3D(const Vector3& pos = Vector3::ZERO, float rotDegrees = 0.0f, const Vector3& scale = Vector3::ONE, Transform3D* parent = nullptr);
    Transform3D(const Transform3D& other);
    ~Transform3D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    Transform3D* GetParent() { return m_parent; };
    void AddChild(Transform3D* child);
    void SetParent(Transform3D* parent);
    void RemoveChild(Transform3D* child);
    void DropChildrenInPlace(); //Unparents all children, moving their positions to your local offset. This prevents them from going to 0,0 by default.
    void RemoveParent();

    Transform3D& operator= (const Transform3D& other);

public:
    //GETTERS/////////////////////////////////////////////////////////////////////
    Vector3 GetWorldPosition() const;
    float GetWorldRotationDegrees() const;
    Vector3 GetWorldScale() const;
    Vector3 GetLocalPosition() const;
    float GetLocalRotationDegrees() const;
    Vector3 GetLocalScale() const;

    //SETTERS/////////////////////////////////////////////////////////////////////
    void SetPosition(const Vector3& position);
    void SetRotationDegrees(float rotationDegrees);
    void SetScale(const Vector3& scale);

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    inline void IgnoreParentTranslation() { m_applyParentTranslation = false; };
    inline void IgnoreParentRotation() { m_applyParentRotation = false; };
    inline void IgnoreParentScale() { m_applyParentScale = false; };
    inline void ApplyParentTranslation() { m_applyParentTranslation = true; };
    inline void ApplyParentRotation() { m_applyParentRotation = true; };
    inline void ApplyParentScale() { m_applyParentScale = true; };

private:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Transform3D*> m_children;
    Vector3 m_position;
    Vector3 m_scale;
    Transform3D* m_parent = nullptr;
    float m_rotationDegrees;
    bool m_applyParentTranslation = true;
    bool m_applyParentRotation = true;
    bool m_applyParentScale = true;
};
