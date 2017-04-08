#include "Engine/Renderer/AABB2.hpp"
#include <cmath>

const AABB2 AABB2::ZERO_TO_ONE = AABB2(Vector2::ZERO, Vector2::ONE);
const AABB2 AABB2::INVALID = AABB2(Vector2::ONE, -Vector2::ONE);

//-----------------------------------------------------------------------------------
AABB2::AABB2()
{
}

//-----------------------------------------------------------------------------------
AABB2::AABB2(const Vector2& Mins, const Vector2& Maxs) : mins(Mins), maxs(Maxs)
{

}

//-----------------------------------------------------------------------------------
AABB2::~AABB2()
{
}

//-----------------------------------------------------------------------------------
bool AABB2::IsPointInside(const Vector2& point) const
{
    return (point.x < maxs.x && point.y < maxs.y && point.x > mins.x && point.y > mins.y);
}

//-----------------------------------------------------------------------------------
bool AABB2::IsPointOnOrInside(const Vector2& point) const
{
    return (point.x <= maxs.x && point.y <= maxs.y && point.x >= mins.x && point.y >= mins.y);
}

//-----------------------------------------------------------------------------------
bool AABB2::IsValid(const AABB2& aabb2ToValidate)
{
    return (aabb2ToValidate.mins.x < aabb2ToValidate.maxs.x && aabb2ToValidate.mins.y < aabb2ToValidate.maxs.y);
}

//-----------------------------------------------------------------------------------
bool AABB2::IsIntersecting(const Vector2& position, const float& radius) const
{
    AABB2 minkowskiBox = AABB2(Vector2(mins.x - radius, mins.y - radius), Vector2(maxs.x + radius, maxs.y + radius));
    return minkowskiBox.IsPointInside(position);
}

//-----------------------------------------------------------------------------------
bool AABB2::IsIntersecting(const AABB2& other) const
{
    return IsValid(GetIntersectingAABB2(*this, other));
}

//-----------------------------------------------------------------------------------
Vector2 AABB2::GetSmallestOutToInResolutionVector(const Vector2& pointOutside)
{
    float dispXMins = pointOutside.x - mins.x;
    float dispXMaxs = pointOutside.x - maxs.x;
    float dispYMins = pointOutside.y - mins.y;
    float dispYMaxs = pointOutside.y - maxs.y;
    bool xMaxsIsSmaller = abs(dispXMaxs) < abs(dispXMins);
    bool yMaxsIsSmaller = abs(dispYMaxs) < abs(dispYMins);
    Vector2 displacement = Vector2(xMaxsIsSmaller ? dispXMaxs : dispXMins, yMaxsIsSmaller ? dispYMaxs : dispYMins);
    if (pointOutside.x < maxs.x && pointOutside.x > mins.x)
    {
        displacement.x = 0.0f;
    }
    if (pointOutside.y < maxs.y && pointOutside.y > mins.y)
    {
        displacement.y = 0.0f;
    }
    return -1.0f * displacement;
}

//-----------------------------------------------------------------------------------
Vector2 AABB2::GetSmallestInToOutResolutionVector(const Vector2& pointInside)
{
    float dispXMins = mins.x - pointInside.x;
    float dispXMaxs = maxs.x - pointInside.x;
    float dispYMins = mins.y - pointInside.y;
    float dispYMaxs = maxs.y - pointInside.y;
    bool xMaxsIsSmaller = abs(dispXMaxs) < abs(dispXMins);
    bool yMaxsIsSmaller = abs(dispYMaxs) < abs(dispYMins);
    return Vector2(xMaxsIsSmaller ? dispXMaxs : dispXMins, yMaxsIsSmaller ? dispYMaxs : dispYMins);
}

//-----------------------------------------------------------------------------------
AABB2 AABB2::GetIntersectingAABB2(const AABB2& first, const AABB2& second)
{
    //todo: do invalid checks here.
    float minsX = first.mins.x > second.mins.x ? first.mins.x : second.mins.x;
    float minsY = first.mins.y > second.mins.y ? first.mins.y : second.mins.y;
    float maxsX = first.maxs.x < second.maxs.x ? first.maxs.x : second.maxs.x;
    float maxsY = first.maxs.y < second.maxs.y ? first.maxs.y : second.maxs.y;
    return AABB2(Vector2(minsX, minsY), Vector2(maxsX, maxsY));
}

//-----------------------------------------------------------------------------------
AABB2 AABB2::GetEncompassingAABB2(const AABB2& first, const AABB2& second)
{
    //If both are invalid, we should return something invalid anyway
    if (!IsValid(first))
    {
        return second;
    }
    if (!IsValid(second))
    {
        return first;
    }
    float minsX = first.mins.x < second.mins.x ? first.mins.x : second.mins.x;
    float minsY = first.mins.y < second.mins.y ? first.mins.y : second.mins.y;
    float maxsX = first.maxs.x > second.maxs.x ? first.maxs.x : second.maxs.x;
    float maxsY = first.maxs.y > second.maxs.y ? first.maxs.y : second.maxs.y;
    return AABB2(Vector2(minsX, minsY), Vector2(maxsX, maxsY));
}

//-----------------------------------------------------------------------------------
AABB2 AABB2::CreateMinkowskiBox(const AABB2& original, float radius)
{
    return AABB2(Vector2(original.mins.x - radius, original.mins.y - radius), Vector2(original.maxs.x + radius, original.maxs.y + radius));
}

//-----------------------------------------------------------------------------------
Vector2 AABB2::GetRandomPointInside()
{
    Vector2 range = maxs - mins;
    return mins + Vector2(MathUtils::GetRandomFloatFromZeroTo(range.x), MathUtils::GetRandomFloatFromZeroTo(range.y));
}

//-----------------------------------------------------------------------------------
AABB2& AABB2::operator+=(const Vector2& rhs)
{
    this->mins += rhs;
    this->maxs += rhs;
    return *this;
}

//-----------------------------------------------------------------------------------
float AABB2::GetWidth() const
{
    return maxs.x - mins.x;
}

//-----------------------------------------------------------------------------------
float AABB2::GetHeight() const
{
    return maxs.y - mins.y;
}
