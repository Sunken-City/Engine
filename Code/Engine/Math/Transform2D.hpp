#pragma once

#include <vector>
#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------------
class Transform2D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Transform2D(const Vector2& pos = Vector2::ZERO, float rotDegrees = 0.0f, const Vector2& scale = Vector2::ONE, Transform2D* parent = nullptr);

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void AddChild(Transform2D* child);
    void SetParent(Transform2D* parent);

    //GETTERS/////////////////////////////////////////////////////////////////////
    Vector2 GetPosition();
    float GetRotationDegrees();
    Vector2 GetScale();

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