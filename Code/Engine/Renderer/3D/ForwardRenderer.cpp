#include "ForwardRenderer.hpp"
#include "Scene3D.hpp"
#include "Camera3D.hpp"

ForwardRenderer* ForwardRenderer::instance = nullptr;

//-----------------------------------------------------------------------------------
ForwardRenderer::ForwardRenderer()
{
    m_scenes.push_back(new Scene3D());
    m_cameras.push_back(new Camera3D());
}

//-----------------------------------------------------------------------------------
ForwardRenderer::~ForwardRenderer()
{
    for (Scene3D* scene : m_scenes)
    {
        delete scene;
    }
    m_scenes.clear();
    for (Camera3D* camera : m_cameras)
    {
        delete camera;
    }
    m_cameras.clear();
}

//-----------------------------------------------------------------------------------
void ForwardRenderer::Update(float deltaSeconds)
{
    for (Scene3D* scene : m_scenes)
    {
        scene->Update(deltaSeconds);
    }
    for (Camera3D* camera : m_cameras)
    {
        camera->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void ForwardRenderer::Render()
{
    for (Scene3D* scene : m_scenes)
    {
        scene->Render();
    }
    for (Camera3D* camera : m_cameras)
    {
        camera->Render();
    }
}

