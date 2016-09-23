#pragma once
#include "Engine/Components/Component.hpp"
#include "Engine/Math/Vector3.hpp"

class Transform3D : Component
{
public:
    Transform3D();
    ~Transform3D();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Vector3 position;
    float rotationDegrees;
    Vector3 scale;

    Transform3D* parent;
    Transform3D* children;
    Transform3D* prev;
    Transform3D* next;
};