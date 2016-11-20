#pragma once
#include <string>
#include <map>

//-----------------------------------------------------------------------------------
enum PropertyGetResult
{
    PGR_SUCCESS,
    PGR_FAILED_WRONG_TYPE,
    PGR_FAILED_NO_SUCH_PROPERTY,
    PGR_FAILED_NO_PROPERTIES,
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
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    NamedProperties();
    ~NamedProperties();

    //-----------------------------------------------------------------------------------
    template<typename T>
    NamedProperties(const std::string& propertyName1, const T& propertyValue1)
    {
        Set(propertyName1, propertyValue1);
    }

    //-----------------------------------------------------------------------------------
    template<typename T, typename U>
    NamedProperties(const std::string& propertyName1, const T& propertyValue1, const std::string& propertyName2, const U& propertyValue2)
    {
        Set(propertyName1, propertyValue1);
        Set(propertyName2, propertyValue2);
    }

    //-----------------------------------------------------------------------------------
    template<typename T, typename U, typename V>
    NamedProperties(const std::string& propertyName1, const T& propertyValue1, const std::string& propertyName2, const U& propertyValue2, const std::string& propertyName3, const V& propertyValue3)
    {
        Set(propertyName1, propertyValue1);
        Set(propertyName2, propertyValue2);
        Set(propertyName3, propertyValue3);
    }

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    //-----------------------------------------------------------------------------------
    template<typename T>
    PropertyGetResult Get(const std::string& propertyName, T& outPropertyValue)
    {
        if (m_properties.size() == 0)
        {
            return PropertyGetResult::PGR_FAILED_NO_PROPERTIES;
        }
        auto result = m_properties.find(propertyName);
        if (result == m_properties.end())
        {
            return PropertyGetResult::PGR_FAILED_NO_SUCH_PROPERTY;
        }

        NamedPropertyBase* property = result->second;
        TypedNameProperty<T>* typedProperty = dynamic_cast<TypedNameProperty<T>*>(property);
        if (!typedProperty || (typeid(outPropertyValue) != typeid(typedProperty->m_data)))
        {
            return PropertyGetResult::PGR_FAILED_WRONG_TYPE;
        }
        outPropertyValue = typedProperty->m_data;

        return PropertyGetResult::PGR_SUCCESS;
    };

    //-----------------------------------------------------------------------------------
    template<typename T>
    PropertySetResult Set(const std::string& propertyName, const T& propertyValue, bool changeTypeIfDifferent = true)
    {
        PropertySetResult result = PSR_SUCCESS;
        if (m_properties.find(propertyName) != m_properties.end())
        {
            if (changeTypeIfDifferent)
            {
                result = PSR_SUCCESS_EXISTED;
            }
            else
            {
                result = PSR_FAILED_DIFF_TYPE;
                return result;
            }
        }
        m_properties[propertyName] = static_cast<NamedPropertyBase*>(new TypedNameProperty<T>(propertyValue));
        return result;
    };

    PropertySetResult Set(const std::string& propertyName, const char* propertyValue, bool changeTypeIfDifferent = true);
    bool Remove(const std::string& propertyName);

    //VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<std::string, NamedPropertyBase*> m_properties;
    static NamedProperties NONE;
};
