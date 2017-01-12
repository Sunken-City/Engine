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
    void SetModelMatrix(const Matrix4x4& model);
    void SetDiffuseTexture(Texture* diffuseTexture);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Mesh m_mesh;
    MeshBuilder m_builder;
private:
    MeshRenderer m_renderer;
};