#pragma once
#include <string>
#include <map>

//-----------------------------------------------------------------------------------
enum PropertyGetResult
{
    PGR_SUCCESS,
    PGR_FAILED_WRONG_TYPE,
    PGR_FAILED_NO_SUCH_PROPERTY,
    PGR_FAILED_EMPTY,
    PGR_NUM_RESULTS
};

//-----------------------------------------------------------------------------------
enum PropertySetResult
{
    PSR_SUCCESS,
    PSR_SUCCESS_EXISTED,
    PSR_SUCCESS_CHANGED_TYPE,
    PSR_FAILED_DIFF_TYPE,
    PSR_NUM_RESULTS
};

//-----------------------------------------------------------------------------------
struct NamedPropertyBase
{
    virtual ~NamedPropertyBase() {};
};

//-----------------------------------------------------------------------------------
template<typename T>
struct TypedNameProperty : public NamedPropertyBase
{
    TypedNameProperty(const T& data)
        : m_data(data)
    {}

    virtual ~TypedNameProperty() {};

    T m_data;
};

//-----------------------------------------------------------------------------------
class NamedProperties
{
public:
    template<typename T>
    PropertyGetResult Get(const std::string& propertyName, T& outPropertyValue)
    {
        auto result = m_properties.find(propertyName);
        if (result == m_properties.end())
        {
            return PropertyGetResult::PGR_FAILED_NO_SUCH_PROPERTY;
        }
        NamedPropertyBase* property = result->second;
        TypedNameProperty<T>* typedProperty = dynamic_cast<TypedNameProperty<T>*>(property);
    };

    template<typename T>
    PropertySetResult Set(const std::string& propertyName, const T& propertyValue, bool changeTypeIfDifferent = true)
    {
        //m_properties[propertyName] = new TypedNameProperty<T>*(propertyValue);
        return PropertySetResult::PSR_SUCCESS;
    };

    void Remove(const std::string& propertyName);

    std::map<std::string, NamedPropertyBase*> m_properties;
};
