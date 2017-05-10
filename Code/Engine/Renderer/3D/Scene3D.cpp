#include "Engine/Renderer/3D/Scene3D.hpp"

//-----------------------------------------------------------------------------------
Scene3D::Scene3D()
{

}

//-----------------------------------------------------------------------------------
Scene3D::~Scene3D()
{

}

//-----------------------------------------------------------------------------------
void Scene3D::Update(float deltaSeconds)
{
    for (Renderable3D* renderable : m_renderables)
    {
        renderable->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void Scene3D::Render() const
{
    for (Renderable3D* renderable : m_renderables)
    {
        renderable->Render();
    }
}
