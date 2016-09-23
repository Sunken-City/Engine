#pragma once
#include <map>

class InputValue;

// CInputMap - tying this shit together
// Acts as a virtual device, but doesn't extend device
class InputMap
{
public:
    InputValue* AddInputValue(std::string const &name);
    InputValue* AddInputValue(std::string const &name, InputValue *other);
    InputValue* FindInputValue(std::string const &name);
    float GetValue(std::string const &name);

    std::map<std::string, InputValue*> m_values;
};