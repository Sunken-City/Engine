#pragma once
#include "..\Math\Vector2.hpp"

class AABB2;

//-----------------------------------------------------------------------------------
struct Dimension
{
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetMinSize(float newMinimum);
    void SetMaxSize(float newMaximum);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    float m_minSize = 0.0f;
    float m_maxSize = 0.0f;
    float m_percentSize = -1.0f;
    bool m_fitWithinParent = false;
    bool m_enlargeParentToFit = false;
};

//-----------------------------------------------------------------------------------
struct Dimensions2D
{
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    AABB2 GetAsAABB2();
    inline Vector2 GetMinSize() { return Vector2(x.m_minSize, y.m_minSize); };
    inline Vector2 GetMaxSize() { return Vector2(x.m_maxSize, y.m_maxSize); };
    inline void SetMinSize(const Vector2& size) { x.SetMinSize(size.x); y.SetMinSize(size.y); };
    inline void SetMaxSize(const Vector2& size) { x.SetMaxSize(size.x); y.SetMaxSize(size.y); };

    ////MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Dimension x;
    Dimension y;
};