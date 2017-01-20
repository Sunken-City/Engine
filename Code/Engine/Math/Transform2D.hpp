#pragma once

#include <vector>
#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------------
class Transform2D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Transform2D(const Vector2& pos = Vector2::ZERO, float rotDegrees = 0.0f, const Vector2& scale = Vector2::ONE, Transform2D* parent = nullptr);
    ~Transform2D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    Transform2D* GetParent() { return m_parent; };
    void AddChild(Transform2D* child);
    void RemoveChild(Transform2D* child);
    void RemoveParent();

private:
    void SetParent(Transform2D* parent);

public:
    //GETTERS/////////////////////////////////////////////////////////////////////
    Vector2 GetWorldPosition();
    float GetWorldRotationDegrees();
    Vector2 GetWorldScale();
    Vector2 GetLocalPosition();
    float GetLocalRotationDegrees();
    Vector2 GetLocalScale();

    //SETTERS/////////////////////////////////////////////////////////////////////
    void SetPosition(const Vector2& position);
    void SetRotationDegrees(float rotationDegrees);
    void SetScale(const Vector2& scale);

private:
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Vector2 m_position;
    float m_rotationDegrees;
    Vector2 m_scale;
    Transform2D* m_parent = nullptr;
    std::vector<Transform2D*> m_children;
};