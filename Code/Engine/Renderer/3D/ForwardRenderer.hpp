#pragma once
#include <vector>

class Camera3D;
class Scene3D;

class ForwardRenderer
{
public:
    ForwardRenderer();
    ~ForwardRenderer();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render();
    Scene3D* GetMainScene() { return m_scenes[0]; };
    Camera3D* GetMainCamera() { return m_cameras[0]; };

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static ForwardRenderer* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Scene3D*> m_scenes;
    std::vector<Camera3D*> m_cameras;
};