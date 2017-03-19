#pragma once

class BufferedMeshRenderer;
class AABB2;

typedef unsigned char uchar;

class Renderable2D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Renderable2D(int orderingLayer = 0, bool isEnabled = true);
    virtual ~Renderable2D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void ChangeLayer(int layer);
    void Enable();
    void Disable();
    virtual void Update(float deltaSeconds) = 0;
    virtual void Render(BufferedMeshRenderer& renderer);
    virtual AABB2 GetBounds();
    virtual bool IsCullable() { return true; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Renderable2D* prev;
    Renderable2D* next;
    int m_orderingLayer; //Drawing order is ordered by layer, smallest to largest
    bool m_isEnabled; //If disabled - does not get rendered
    bool m_isDead = false;
    uchar m_viewableBy;
};