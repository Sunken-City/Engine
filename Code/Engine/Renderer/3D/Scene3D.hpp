#pragma once
#include "Renderable3D.hpp"
#include <vector>

class Light;

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
    void UnregisterRenderable(Renderable3D* renderable);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr int MAX_NUM_LIGHTS = 8;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Renderable3D*> m_renderables;
    Light* lights[MAX_NUM_LIGHTS];
};