#include "Engine/Math/Transform2D.hpp"

//-----------------------------------------------------------------------------------
Transform2D::Transform2D(const Vector2& pos, float rotDegrees, const Vector2& scaleVal)
    : position(pos)
    , rotationDegrees(rotDegrees)
    , scale(scaleVal)
{

}

