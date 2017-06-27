#pragma once

class Vector2;
class Vector2Int;
class Vector3;
class Vector3Int;
class Vector4;

#define BIT(x) (1<<(x))
typedef unsigned char uchar;
typedef unsigned char byte;

class MathUtils
{
public:
    //DISTANCE CALCULATION//////////////////////////////////////////////////////////////////////////
    static float CalcDistanceBetweenPoints(const Vector2& pos1, const Vector2& pos2);
    static float CalcDistSquaredBetweenPoints(const Vector2& pos1, const Vector2& pos2);
    static float CalcDistSquaredBetweenPoints(const Vector2Int& pos1, const Vector2Int& pos2);
    static float CalcDistSquaredBetweenPoints(const Vector3& pos1, const Vector3& pos2);

    //CLAMPING AND MAPPING//////////////////////////////////////////////////////////////////////////
    static float RangeMap(float inValue, float min1, float max1, float min2, float max2);
    static float Clamp(float inputValue);
    static float Clamp(float inputValue, float min, float max);
    static Vector3 Clamp(const Vector3& inputValue, float min, float max);
    static Vector3 RemoveDirectionalComponent(const Vector3& original, const Vector3& directionToStripOut);

    //DISK MATH//////////////////////////////////////////////////////////////////////////
    static bool DoDiscsOverlap(const Vector2& center1, float radius1, const Vector2& center2, float radius2);
    static bool IsPointInDisk(const Vector2& point, const Vector2&  center, float radius);
    static float CalcShortestAngularDisplacement(float fromDegrees, float toDegrees);

    //RANDOM//////////////////////////////////////////////////////////////////////////
    static int GetRandomInt(int minimum, int maximum);
    static float GetRandom();
    static float GetRandomFloat(float minimum, float maximum);
    static float GetRandomFloatInRange(float minimumInclusive, float maximumInclusive);
    static int GetRandomIntFromZeroTo(int maximum);
    static float GetRandomFloatFromZeroTo(float maximum);
    static bool CoinFlip();
    static Vector2 GetRandomVectorInCircle(float radius);

    //INTERPOLATION//////////////////////////////////////////////////////////////////////////
    static float SmoothStep(float inputZeroToOne);
    static float SmoothStart2(float inputZeroToOne);
    static float SmoothStop2(float inputZeroToOne);
    static float EaseInOut2(float inputZeroToOne);
    static int LerpInt(float fraction, int initialValue, int endValue);
    static float Lerp(float fraction, float initialValue, float endValue);
    static Vector2 Lerp(float fraction, const Vector2& initialValue, const Vector2& endValue);
    static Vector3 Lerp(float fraction, const Vector3& initialValue, const Vector3& endValue);
    static Vector2 GetRandomDirectionVector();
    //Returns Vector2(R, theta);
    static Vector2 MakePolar(const Vector2& xyVector);

    //Here for compilation reasons :T
    static float SinDegrees(float input);

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const float PI;
    static const float TWO_PI;
    static const float HALF_PI;
};

//Global functions. I'm planning on moving everything out of the MathUtils class as soon as I have time to refactor all the code the change will break.

//BIT MANIPULATION//////////////////////////////////////////////////////////////////////////
void SetBit(uchar& bitFlags, uchar bitMask);
bool IsBitSet(uchar bitFlags, uchar bitMask);
void ClearBit(uchar& bitFlags, uchar bitMask);
void SetBitUint(unsigned int& bitFlags, uchar bitMask);
bool IsBitSetUint(unsigned int bitFlags, uchar bitMask);
void ClearBitUint(unsigned int& bitFlags, uchar bitMask);

//TRIGONOMETRY//////////////////////////////////////////////////////////////////////////
float Dot(const Vector2& a, const Vector2& b);
float Dot(const Vector3& a, const Vector3& b);
float Dot(const Vector4& a, const Vector4& b);
float CosDegrees(float input);
float DegreesToRadians(float degrees);
float RadiansToDegrees(float radians);

int Mod(int a, int b);
float Clamp01(float input);

//INTERPOLATION//////////////////////////////////////////////////////////////////////////
template <typename T>
inline T Lerp(const float fraction, const T& initialValue, const T& endValue)
{
    return initialValue + fraction * (endValue - initialValue);
}

//MIN/MAX/////////////////////////////////////////////////////////////////////
template <typename T>
inline T Max(const T& first, const T& second)
{
    if (first > second)
    {
        return first;
    }
    else
    {
        return second;
    }
}

//-----------------------------------------------------------------------------------
template <typename T>
inline T Min(const T& first, const T& second)
{
    if (first < second)
    {
        return first;
    }
    else
    {
        return second;
    }
}


//-----------------------------------------------------------------------------------
template <typename T>
inline T Clamp(const T& inputValue, const T& min, const T& max)
{
    if (inputValue < min)
    {
        return min;
    }
    else if (inputValue > max)
    {
        return max;
    }
    else
    {
        return inputValue;
    }
}

//---------------------------------------------------------------------------------
inline uchar ClampedAddition(uchar first, uchar second)
{
    uchar sum = first + second;
    return sum < first ? 0xFF : sum;
}

//---------------------------------------------------------------------------------
inline uchar ClampedSubtraction(uchar first, uchar second)
{
    uchar sum = first - second;
    return sum > first ? 0x00 : sum;
}

// 
// #include "Engine/Math/Vector4.hpp"
// 
// //-----------------------------------------------------------------------------------
// template <>
// Vector4 Min(const Vector4& first, const Vector4& second)
// {
//     return Vector4(Min(first.x, second.x), Min(first.y, second.y), Min(first.z, second.z), Min(first.w, second.w));
// }
// 
// //-----------------------------------------------------------------------------------
// template <>
// Vector4 Max(const Vector4& first, const Vector4& second)
// {
//     return Vector4(Max(first.x, second.x), Max(first.y, second.y), Max(first.z, second.z), Max(first.w, second.w));
// }


//RANGE/////////////////////////////////////////////////////////////////////
template <typename T>
class Range
{
public:
    //-----------------------------------------------------------------------------------
    //Added this default constructor in order to use Get in named properties, however, you should always give the range a value.
    Range()
    {};

    //-----------------------------------------------------------------------------------
    Range(T const &a)
        : minValue(a)
        , maxValue(a) {}

    //-----------------------------------------------------------------------------------
    Range(const T& a, const T& b)
    {
        minValue = Min(a, b);
        maxValue = Max(a, b);
    }

    //-----------------------------------------------------------------------------------
    T Get(const float t) const
    {
        return Lerp(t, minValue, maxValue);
    }

    //-----------------------------------------------------------------------------------
    T GetRandom() const
    {
        return Get(MathUtils::GetRandomFloatFromZeroTo(1.0f));
    }

    //-----------------------------------------------------------------------------------
    //This is used for assigning a value from a random one in a range. Kind of spooky, but now you know what it's doing!
    operator T()
    {
        return GetRandom();
    }

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    T minValue;
    T maxValue;
};