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

//-----------------------------------------------------------------------------------
void Scene3D::UnregisterRenderable(Renderable3D* renderable)
{
    for (unsigned int i = 0; i < m_renderables.size(); ++i)
    {
        if (m_renderables[i] == renderable)
        {
            m_renderables[i] = m_renderables[m_renderables.size() - 1];
            m_renderables.pop_back();
            return;
        }
    }
    ERROR_RECOVERABLE("Couldn't find renderable to unregister");
}
