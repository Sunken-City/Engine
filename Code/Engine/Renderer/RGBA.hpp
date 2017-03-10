#pragma once
#include <string>
#include "Engine\Math\MathUtils.hpp"

typedef unsigned char uchar;

class Vector4;

class RGBA

{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    RGBA();
    RGBA(float Red, float Green, float Blue);
    RGBA(float Red, float Green, float Blue, float Alpha);
    RGBA(unsigned int hexColor);
    RGBA(const Vector4& color);
    ~RGBA();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    static inline unsigned char GetRed(unsigned int hexColor);
    static inline unsigned char GetGreen(unsigned int hexColor);
    static inline unsigned char GetBlue(unsigned int hexColor);
    static RGBA CreateFromString(const std::string& textColor);
    static inline RGBA CreateFromUChars(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
    static Vector4 GetRandomVec4();
    static RGBA GetRandom();
    inline RGBA WithoutAlpha();
    inline unsigned int ToUnsignedInt() const;
    void SetAlphaFloat(float alpha);
    Vector4 ToVec4() const;
    RGBA GetInverse() const;
    float GetAlphaFloat() const;

    //OPERATORS//////////////////////////////////////////////////////////////////////////
    //Damp an RGBA color by a single value. Doesn't affect Alpha.
    RGBA& operator-=(unsigned char rhs)
    {
        this->red -= rhs;
        this->green -= rhs;
        this->blue -= rhs;
        return *this;
    }

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;

    //CONSTANTS//////////////////////////////////////////////////////////////////////////
    static const RGBA WHITE;
    static const RGBA BLACK;
    static const RGBA RED;
    static const RGBA BLUE;
    static const RGBA GREEN;
    static const RGBA MAGENTA;
    static const RGBA YELLOW;
    static const RGBA CYAN;
    static const RGBA ORANGE;
    static const RGBA CORNFLOWER_BLUE;
    static const RGBA TURQUOISE;
    static const RGBA SLATE;
    static const RGBA FOREST_GREEN;
    static const RGBA SEA_GREEN;
    static const RGBA KHAKI;
    static const RGBA GOLD;
    static const RGBA CHOCOLATE;
    static const RGBA SADDLE_BROWN;
    static const RGBA MAROON;
    static const RGBA KINDA_GRAY;
    static const RGBA GRAY;
    static const RGBA VERY_GRAY;
    static const RGBA VAPORWAVE;
    static const RGBA DEADBEEF;
    static const RGBA FEEDFACE;
    static const RGBA DISEASED;
    static const RGBA BADDAD;
    static const RGBA CERULEAN;
    static const RGBA GBBLACK;
    static const RGBA GBLIGHTGREEN;
    static const RGBA GBDARKGREEN;
    static const RGBA GBWHITE;
    static const RGBA MUDKIP_BLUE;
    static const RGBA MUDKIP_ORANGE;
    static const RGBA JOLTIK_YELLOW;
    static const RGBA JOLTIK_PURPLE;
    static const RGBA JOLTIK_DARK_PURPLE;
    static const RGBA CLEAR;
    static const int SHIFT_RED = 6 * 4;
    static const int SHIFT_GREEN = 4 * 4;
    static const int SHIFT_BLUE = 2 * 4;
    static const int RED_MASK = 0xFF000000;
    static const int GREEN_MASK = 0x00FF0000;
    static const int BLUE_MASK = 0x0000FF00;
    static const int ALPHA_MASK = 0x000000FF;
    
};

//---------------------------------------------------------------------------
//Damp an RGBA color by a single value. Doesn't affect Alpha.
inline RGBA operator-(RGBA lhs, unsigned char rhs)
{
    lhs -= rhs;
    return lhs;
}

//---------------------------------------------------------------------------
inline RGBA operator-(const RGBA& lhs, const RGBA& rhs)
{
    RGBA result;
    result.red   = ClampedSubtraction(lhs.red, rhs.red);
    result.green = ClampedSubtraction(lhs.green, rhs.green);
    result.blue  = ClampedSubtraction(lhs.blue, rhs.blue);
    result.alpha = ClampedSubtraction(lhs.alpha, rhs.alpha);
    return result;
}

//---------------------------------------------------------------------------
inline RGBA operator+(const RGBA& lhs, const RGBA& rhs)
{
    RGBA result;
    result.red   = ClampedAddition(lhs.red, rhs.red);
    result.green = ClampedAddition(lhs.green, rhs.green);
    result.blue  = ClampedAddition(lhs.blue, rhs.blue);
    result.alpha = ClampedAddition(lhs.alpha, rhs.alpha);
    return result;
}

//---------------------------------------------------------------------------
inline RGBA RGBA::CreateFromUChars(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    return RGBA((red << RGBA::SHIFT_RED) + (green << RGBA::SHIFT_GREEN) + (blue << RGBA::SHIFT_BLUE) + alpha);
}

//---------------------------------------------------------------------------
inline unsigned char RGBA::GetRed(unsigned int hexColor)
{
    return (unsigned char)((hexColor & RED_MASK) >> SHIFT_RED);
}

//---------------------------------------------------------------------------
inline unsigned char RGBA::GetGreen(unsigned int hexColor)
{
    return (unsigned char)((hexColor & GREEN_MASK) >> SHIFT_GREEN);
}

//---------------------------------------------------------------------------
inline unsigned char RGBA::GetBlue(unsigned int hexColor)
{
    return (unsigned char)((hexColor & BLUE_MASK) >> SHIFT_BLUE);
}

//---------------------------------------------------------------------------
inline bool operator==(const RGBA& lhs, const RGBA& rhs)
{ 
    return (lhs.red == rhs.red) && (lhs.green == rhs.green) && (lhs.blue == rhs.blue) && (lhs.alpha == rhs.alpha); 
}

//---------------------------------------------------------------------------
inline bool operator!=(const RGBA& lhs, const RGBA& rhs)
{
    return !(lhs == rhs);
}

//-----------------------------------------------------------------------------------
inline RGBA RGBA::WithoutAlpha()
{
    return RGBA((red << RGBA::SHIFT_RED) + (green << RGBA::SHIFT_GREEN) + (blue << RGBA::SHIFT_BLUE));
}

//-----------------------------------------------------------------------------------
inline unsigned int RGBA::ToUnsignedInt() const
{
    return (red << RGBA::SHIFT_RED) + (green << RGBA::SHIFT_GREEN) + (blue << RGBA::SHIFT_BLUE) + alpha;
}