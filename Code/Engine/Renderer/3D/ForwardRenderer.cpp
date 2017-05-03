#include "ForwardRenderer.hpp"
#include "Scene3D.hpp"

ForwardRenderer* ForwardRenderer::instance = nullptr;

//-----------------------------------------------------------------------------------
ForwardRenderer::ForwardRenderer()
{
    m_scenes.push_back(new Scene3D());
}

//-----------------------------------------------------------------------------------
ForwardRenderer::~ForwardRenderer()
{
    for (Scene3D* scene : m_scenes)
    {
        delete scene;
    }
    m_scenes.clear();
}

//-----------------------------------------------------------------------------------
void ForwardRenderer::Update(float deltaSeconds)
{
    for (Scene3D* scene : m_scenes)
    {
        scene->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void ForwardRenderer::Render()
{
    for (Scene3D* scene : m_scenes)
    {
        scene->Render();
    }
}

