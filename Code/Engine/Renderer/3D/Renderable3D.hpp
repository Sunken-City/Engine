#pragma once
#include "..\MeshRenderer.hpp"
#include "..\..\Math\Transform3D.hpp"

class Renderable3D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Renderable3D();
    ~Renderable3D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render() const;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    MeshRenderer m_meshRenderer;
    Transform3D m_transform;
};