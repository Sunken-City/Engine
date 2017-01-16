#include "Engine/Core/Events/NamedProperties.hpp"

NamedProperties NamedProperties::NONE;

//-----------------------------------------------------------------------------------
NamedProperties::NamedProperties()
{

}

//-----------------------------------------------------------------------------------
NamedProperties::~NamedProperties()
{
    for (auto keyValuePair : m_properties)
    {
        delete keyValuePair.second;
    }
    m_properties.clear();
}

//-----------------------------------------------------------------------------------
PropertySetResult NamedProperties::Set(const std::string& propertyName, std::string propertyValue, bool changeTypeIfDifferent)
{
    PropertySetResult result = PSR_SUCCESS;
    auto foundPair = m_properties.find(propertyName);
    if (foundPair != m_properties.end())
    {
        if (!m_neverChangeTypeIfDifferent && changeTypeIfDifferent)
        {
            result = PSR_SUCCESS_EXISTED;
            delete (*foundPair).second;
        }
        else
        {
            result = PSR_FAILED_DIFF_TYPE;
            ERROR_RECOVERABLE(Stringf("Attempted to Set %s to a different type when it wasn't allowed.", propertyName.c_str()));
            return result;
        }
    }
    m_properties[propertyName] = static_cast<NamedPropertyBase*>(new TypedNameProperty<std::string>(propertyValue));
    return result;
}

//-----------------------------------------------------------------------------------
PropertyGetResult NamedProperties::Get(const std::string& propertyName, std::string& outPropertyValue)
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
    TypedNameProperty<std::string>* typedProperty = dynamic_cast<TypedNameProperty<std::string>*>(property);
    if (!typedProperty)
    {
        return PropertyGetResult::PGR_FAILED_WRONG_TYPE;
    }
    outPropertyValue = std::string(typedProperty->m_data);

    return PropertyGetResult::PGR_SUCCESS;

}

//-----------------------------------------------------------------------------------
bool NamedProperties::Remove(const std::string& propertyName)
{
    auto foundPair = m_properties.find(propertyName);
    if (foundPair != m_properties.end())
    {
        delete foundPair->second;
        m_properties.erase(foundPair);
        return true;
    }
    else
    {
        return false;
    }
}

