#include "Engine/Renderer/AABB2.hpp"
#include <cmath>

const AABB2 AABB2::ZERO_TO_ONE = AABB2(Vector2::ZERO, Vector2::ONE);

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
bool AABB2::IsIntersecting(const AABB2& other) const
{
    return IsValid(GetIntersectingAABB2(*this, other));
}

//-----------------------------------------------------------------------------------
AABB2 AABB2::GetIntersectingAABB2(const AABB2& first, const AABB2& second)
{
    float minsX = first.mins.x > second.mins.x ? first.mins.x : second.mins.x;
    float minsY = first.mins.y > second.mins.y ? first.mins.y : second.mins.y;
    float maxsX = first.maxs.x < second.maxs.x ? first.maxs.x : second.maxs.x;
    float maxsY = first.maxs.y < second.maxs.y ? first.maxs.y : second.maxs.y;
    return AABB2(Vector2(minsX, minsY), Vector2(maxsX, maxsY));
}

//-----------------------------------------------------------------------------------
Vector2 AABB2::GetRandomPointInside()
{
    Vector2 range = maxs - mins;
    return mins + Vector2(MathUtils::GetRandomFloatFromZeroTo(range.x), MathUtils::GetRandomFloatFromZeroTo(range.y));
}

//-----------------------------------------------------------------------------------
// Vector2 AABB2::GetCollisionResolution(const AABB2& first, const AABB2& second)
// {
//     Vector2 resolutionVector = Vector2::ZERO;
//     AABB2 workingBounds = first;
//     while (workingBounds.IsIntersecting(second))
//     {
//         if (!second.IsPointOnOrInside(workingBounds.maxs))
//         {
//             Vector2 difference = workingBounds.maxs - second.mins;
//             if (fabs(difference.x) < fabs(difference.y))
//             {
//                 resolutionVector += Vector2(difference.x, 0.0f);
//                 workingBounds += Vector2(difference.x, 0.0f);
//             }
//             else
//             {
//                 resolutionVector += Vector2(difference.y, 0.0f);
//                 workingBounds += Vector2(difference.y, 0.0f);
//             }
//         }
//     }
//     return resolutionVector;
// }

//-----------------------------------------------------------------------------------
AABB2& AABB2::operator+=(const Vector2& rhs)
{
    this->mins += rhs;
    this->maxs += rhs;
    return *this;
}