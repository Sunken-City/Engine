#include "Renderable3D.hpp"

//-----------------------------------------------------------------------------------
Renderable3D::Renderable3D()
{
}

//-----------------------------------------------------------------------------------
Renderable3D::Renderable3D(Mesh* mesh, Material* material)
    : m_meshRenderer(mesh, material)
{

}

//-----------------------------------------------------------------------------------
Renderable3D::~Renderable3D()
{
}

//-----------------------------------------------------------------------------------
void Renderable3D::Update(float deltaSeconds)
{
    
}

//-----------------------------------------------------------------------------------
void Renderable3D::Render()
{
    m_meshRenderer.SetModelMatrix(m_transform.GetModelMatrix());
    m_meshRenderer.Render();
}
