#pragma once
#include <string>

//-----------------------------------------------------------------------------------
enum PropertyGetResult
{
    PGR_SUCCESS,
    PGR_FAILED_WRONG_TYPE,
    PGR_FAILED_NOT_PRESENT,
    PGR_FAILED_EMPTY,
    NUM_RESULTS
};

//-----------------------------------------------------------------------------------
enum PropertySetResult
{
    PSR_SUCCESS,
    PSR_SUCCESS_EXISTED,
    PSR_SUCCESS_CHANGED_TYPE,
    PSR_FAILED_DIFF_TYPE,
    NUM_RESULTS
};

//-----------------------------------------------------------------------------------
class NamedProperties
{
    template<typename T>
    PropertyGetResult Get(const std::string& propertyName, T& outPropertyValue)
    {

    };

    template<typename T>
    PropertySetResult Set(const std::string& propertyName, const T& propertyValue, bool changeTypeIfDifferent = true)
    {

    };

    void Remove(const std::string& propertyName);
};
