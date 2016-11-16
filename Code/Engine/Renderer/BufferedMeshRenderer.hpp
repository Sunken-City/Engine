#pragma once
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

class BufferedMeshRenderer
{
public:
    ////CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    BufferedMeshRenderer();
    ~BufferedMeshRenderer();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
private:
    MeshRenderer m_renderer;
    MeshBuilder m_builder;
};