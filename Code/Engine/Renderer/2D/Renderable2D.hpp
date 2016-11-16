#pragma once
#include "..\MeshBuilder.hpp"

class Renderable2D
{
public:
    ////CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Renderable2D(int orderingLayer = 0, bool isEnabled = true);
    ~Renderable2D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void ChangeLayer(int layer);
    void Enable();
    void Disable();
    virtual void Render(MeshRenderer* renderer, MeshBuilder* builder);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Renderable2D* prev;
    Renderable2D* next;
    int m_orderingLayer; //Drawing order is ordered by layer, smallest to largest
    bool m_isEnabled; //If disabled - does not get rendered
};