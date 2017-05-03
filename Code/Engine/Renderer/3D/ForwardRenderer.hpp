#pragma once
#include <vector>

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

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static ForwardRenderer* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Scene3D*> m_scenes;
};