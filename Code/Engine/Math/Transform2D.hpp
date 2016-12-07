#pragma once

#include "Engine/Math/Vector2.hpp"

class Transform2D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Transform2D(const Vector2& pos = Vector2::ZERO, float rotDegrees = 0.0f, const Vector2& scale = Vector2::ONE);

    //FUNCTIONS/////////////////////////////////////////////////////////////////////


    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Vector2 position;
    float rotationDegrees;
    Vector2 scale;
    Transform2D* parent = nullptr;
};