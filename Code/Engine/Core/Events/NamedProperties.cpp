#include "Engine/Core/Events/NamedProperties.hpp"

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
PropertySetResult NamedProperties::Set(const std::string& propertyName, const char* propertyValue, bool changeTypeIfDifferent)
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
    m_properties[propertyName] = static_cast<NamedPropertyBase*>(new TypedNameProperty<const char*>(propertyValue));
    return result;
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
