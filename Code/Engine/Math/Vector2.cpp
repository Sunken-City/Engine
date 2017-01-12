#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cmath>
#include <float.h>
#include <string>
#include "../Core/StringUtils.hpp"

const Vector2 Vector2::ZERO = Vector2(0.0f, 0.0f);
const Vector2 Vector2::ONE = Vector2(1.0f, 1.0f);
const Vector2 Vector2::UNIT_X = Vector2(1.0f, 0.0f);
const Vector2 Vector2::UNIT_Y = Vector2(0.0f, 1.0f);
const Vector2 Vector2::MAX = Vector2(FLT_MAX, FLT_MAX);

const float Vector2::ZERO_DEGREES_RIGHT = 0.0f;
const float Vector2::ZERO_DEGREES_UP = 90.0f;

//-----------------------------------------------------------------------------------
Vector2::Vector2()
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(float initialValue)
    : x(initialValue)
    , y(initialValue)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(float initialX, float initialY) 
    : x(initialX)
    , y(initialY)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(const Vector2& other) 
    : x(other.x)
    , y(other.y)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(const Vector3& other)
    : x(other.x)
    , y(other.y)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(const Vector4& other)
    : x(other.x)
    , y(other.y)
{
}

//-----------------------------------------------------------------------------------
Vector2::Vector2(const Vector2Int& other)
    : x(static_cast<float>(other.x))
    , y(static_cast<float>(other.y))
{

}

//-----------------------------------------------------------------------------------
void Vector2::SetXY(float newX, float NewY)
{
    x = newX;
    y = NewY;
}

//-----------------------------------------------------------------------------------
float Vector2::CalculateMagnitude() const
{
    return sqrt((x*x) + (y*y));
}

//-----------------------------------------------------------------------------------
void Vector2::ClampMagnitude(float desiredMagnitude)
{
    float currentMagnitude = CalculateMagnitude();
    if (currentMagnitude > desiredMagnitude)
    {
        float scalingFactor = desiredMagnitude / currentMagnitude;
        x *= scalingFactor;
        y *= scalingFactor;
    }
}

//-----------------------------------------------------------------------------------
void Vector2::Normalize()
{
    float len = CalculateMagnitude();
    if (len == 0.f)
    {
        return;
    }
    float scale = 1.0f / len;
    x *= scale;
    y *= scale;
}

//-----------------------------------------------------------------------------------
Vector2 Vector2::GetNorm()
{
    float len = CalculateMagnitude();
    if (len == 0.f)
    {
        return Vector2::ZERO;
    }
    else
    {
        float scale = 1.0f / len;
        return Vector2(x * scale, y * scale);
    }
}

//-----------------------------------------------------------------------------------
Vector2 Vector2::CalculateCorrectionVector(const Vector2& position, const Vector2& goal)
{
    return Vector2(goal.x - position.x, goal.y - position.y);
}

//-----------------------------------------------------------------------------------
Vector2 Vector2::DegreesToDirection(float rotationDegrees, float rotationalOffset)
{
    float degrees = rotationDegrees + rotationalOffset;
    return Vector2(MathUtils::CosDegrees(degrees), MathUtils::SinDegrees(degrees));
}

//-----------------------------------------------------------------------------------
float Vector2::Dot(const Vector2& b) const
{
    return(x * b.x) + (y * b.y);
}

//-----------------------------------------------------------------------------------
float Vector2::Dot(const Vector2& a, const Vector2& b)
{
    return(a.x * b.x) + (a.y * b.y);
}

//-----------------------------------------------------------------------------------
float Vector2::CalculateTheta()
{
    return MathUtils::RadiansToDegrees(atan2(y, x));
}

//-----------------------------------------------------------------------------------
Vector2 Vector2::GetMidpoint(const Vector2& start, const Vector2& end)
{
    Vector2 midpoint;
    midpoint.x = (start.x + end.x) / 2.0f;
    midpoint.y = (start.y + end.y) / 2.0f;
    return midpoint;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator+=(const Vector2& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator-=(const Vector2& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator*=(const Vector2& rhs)
{
    this->x *= rhs.x;
    this->y *= rhs.y;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator/=(const Vector2& rhs)
{
    this->x /= rhs.x;
    this->y /= rhs.y;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator*=(const float& scalarConstant)
{
    this->x *= scalarConstant;
    this->y *= scalarConstant;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2& Vector2::operator/=(const float& scalarConstant)
{
    this->x /= scalarConstant;
    this->y /= scalarConstant;
    return *this;
}

//-----------------------------------------------------------------------------------
Vector2 Vector2::CreateFromString(const char* xmlString)
{
    std::vector<std::string>* components = SplitString(std::string(xmlString), ",");
    Vector2 returnValue = Vector2(std::stof(components->at(0)), std::stof(components->at(1)));
    delete components;
    return returnValue;
}