#pragma once
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

class BufferedMeshRenderer
{
public:
    ////CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    BufferedMeshRenderer();
    ~BufferedMeshRenderer();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetMaterial(Material* newMat);
    void FlushAndRender();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
private:
    Mesh m_mesh;
    MeshRenderer m_renderer;
    MeshBuilder m_builder;
};