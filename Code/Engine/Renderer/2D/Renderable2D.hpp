#pragma once

class Renderable2D
{
public:
    ////CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Renderable2D();
    ~Renderable2D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void ChangeLayer(int layer);
    void Enable();
    void Disable();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Renderable2D* prev;
    Renderable2D* next;
    int m_orderingLayer; //Drawing order is ordered by layer, smallest to largest
    bool m_isEnabled; //If disabled - does not get rendered
};