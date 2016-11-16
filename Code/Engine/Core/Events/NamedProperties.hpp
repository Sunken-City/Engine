#pragma once

//-----------------------------------------------------------------------------------
enum PropertyGetResult
{
    PGR_SUCCESS,
    PGR_WRONG_TYPE,
    PGR_NOT_PRESENT,
    PGR_EMPTY,
    NUM_TYPES
};

//-----------------------------------------------------------------------------------
class NamedProperties
{
    template<typename T>
    PropertyGetResult Get(const std::string& propertyName, T& outPropertyValue)
    {

    };
};
