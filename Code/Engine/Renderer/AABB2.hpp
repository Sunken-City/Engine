#pragma once

#include "Engine/Math/Vector2.hpp"

class AABB2
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    AABB2();
    AABB2(const Vector2& Mins, const Vector2& Maxs);
    ~AABB2();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    bool IsPointInside(const Vector2& point) const;
    bool IsPointOnOrInside(const Vector2& point) const;
    bool IsIntersecting(const AABB2& other) const;
    Vector2 GetRandomPointInside();
    inline Vector2 GetBottomRight() const { return Vector2(maxs.x, mins.y); };
    inline Vector2 GetTopLeft()  const { return Vector2(mins.x, maxs.y); };

    //STATIC FUNCTIONS/////////////////////////////////////////////////////////////////////
    static AABB2 GetIntersectingAABB2(const AABB2& first, const AABB2& second);    
    //static Vector2 GetCollisionResolution(const AABB2& first, const AABB2& second);
    static bool IsValid(const AABB2& aabb2ToValidate);

    //OPERATORS/////////////////////////////////////////////////////////////////////
    AABB2& AABB2::operator+=(const Vector2& rhs);

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static const AABB2 ZERO_TO_ONE;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    Vector2 mins;
    Vector2 maxs;
};