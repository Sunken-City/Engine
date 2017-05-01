#pragma once
#include "Renderable3D.hpp"
#include <vector>

class Scene3D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Scene3D();
    ~Scene3D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render() const;
    void RegisterRenderable(Renderable3D* renderable) { m_renderables.push_back(renderable); };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Renderable3D*> m_renderables;
};