#pragma once
#include "MathUtils.hpp"

class EulerAngles
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    EulerAngles();
    EulerAngles(float initialX, float initialY, float initialZ);
    EulerAngles(const EulerAngles& other);

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    float CalculateMagnitude();
    void SetXYZ(float newX, float newY, float newZ);
    void Normalize();
    inline float GetRollRadiansAboutX() const { return DegreesToRadians(rollDegreesAboutX); };
    inline float GetPitchRadiansAboutY() const { return DegreesToRadians(pitchDegreesAboutY); };
    inline float GetYawRadiansAboutZ() const { return DegreesToRadians(yawDegreesAboutZ); };

    //OPERATORS//////////////////////////////////////////////////////////////////////////
    EulerAngles& operator+=(const EulerAngles& rhs);
    EulerAngles& operator-=(const EulerAngles& rhs);
    EulerAngles& operator*=(const float& scalarConstant);

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    float rollDegreesAboutX;
    float pitchDegreesAboutY;
    float yawDegreesAboutZ;
};

//-------------------------------------------------------------------------------
inline EulerAngles operator+(EulerAngles lhs, const EulerAngles& rhs)
{
    lhs += rhs;
    return lhs;
}

//-------------------------------------------------------------------------------
inline EulerAngles operator-(EulerAngles lhs, const EulerAngles& rhs)
{
    lhs -= rhs;
    return lhs;
}

//-------------------------------------------------------------------------------
inline EulerAngles operator*(EulerAngles lhs, const float& scalarConstant)
{
    lhs *= scalarConstant;
    return lhs;
}

//-------------------------------------------------------------------------------
inline bool operator==(const EulerAngles& lhs, const EulerAngles& rhs)
{ 
    return (lhs.rollDegreesAboutX == rhs.rollDegreesAboutX) && (lhs.pitchDegreesAboutY == rhs.pitchDegreesAboutY) && (lhs.yawDegreesAboutZ == rhs.yawDegreesAboutZ); 
}

//-------------------------------------------------------------------------------
inline bool operator!=(const EulerAngles& lhs, const EulerAngles& rhs)
{
    return !(lhs == rhs);
}