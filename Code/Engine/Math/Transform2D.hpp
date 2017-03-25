#pragma once

#include <vector>
#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------------
class Transform2D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Transform2D(const Vector2& pos = Vector2::ZERO, float rotDegrees = 0.0f, const Vector2& scale = Vector2::ONE, Transform2D* parent = nullptr);
    Transform2D(const Transform2D& other);
    ~Transform2D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    Transform2D* GetParent() { return m_parent; };
    void AddChild(Transform2D* child);
    void SetParent(Transform2D* parent);
    void RemoveChild(Transform2D* child);
    void RemoveParent();

    Transform2D& operator= (const Transform2D& other);
    
public:
    //GETTERS/////////////////////////////////////////////////////////////////////
    Vector2 GetWorldPosition() const;
    float GetWorldRotationDegrees() const;
    Vector2 GetWorldScale() const;
    Vector2 GetLocalPosition() const;
    float GetLocalRotationDegrees() const;
    Vector2 GetLocalScale() const;

    //SETTERS/////////////////////////////////////////////////////////////////////
    void SetPosition(const Vector2& position);
    void SetRotationDegrees(float rotationDegrees);
    void SetScale(const Vector2& scale);

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    inline void IgnoreParentTranslation() { m_applyParentTranslation = false; };
    inline void IgnoreParentRotation() { m_applyParentRotation = false; };
    inline void IgnoreParentScale() { m_applyParentScale = false; };
    inline void ApplyParentTranslation() { m_applyParentTranslation = true; };
    inline void ApplyParentRotation() { m_applyParentRotation = true; };
    inline void ApplyParentScale() { m_applyParentScale = true; };

private:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Transform2D*> m_children;
    Vector2 m_position;
    Vector2 m_scale;
    Transform2D* m_parent = nullptr;
    float m_rotationDegrees;
    bool m_applyParentTranslation = true;
    bool m_applyParentRotation = true;
    bool m_applyParentScale = true;
};