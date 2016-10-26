#pragma once
#include <map>

class InputValue;
class VirtualInputValue;
class InputAxis;
class InputVector2;
class Vector2;

// CInputMap - tying this shit together
// Acts as a virtual device, but doesn't extend device
class InputMap
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    InputMap();
    ~InputMap();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    InputValue* MapInputValue(std::string const &name);
    InputValue* MapInputValue(std::string const &name, InputValue *other);
    InputValue* FindInputValue(std::string const &name);
    float GetValue(std::string const &name);
    bool IsDown(const std::string& name);
    bool IsUp(const std::string& name);
    bool WasJustPressed(const std::string& name);
    bool WasJustReleased(const std::string& name);
    InputAxis* MapInputAxis(const std::string &name);
    InputAxis* MapInputAxis(const std::string& name, InputAxis* inputAxis);
    InputAxis* MapInputAxis(const std::string& name, VirtualInputValue& positiveInput, VirtualInputValue& negativeInput);
    InputAxis* FindInputAxis(std::string const &name);
    Vector2 GetVector2(const std::string& xName, const std::string& yName);
    void Clear();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::map<std::string, InputValue*> m_values;
    std::map<std::string, InputAxis*> m_axies;
    std::map<std::string, InputVector2*> m_vectors;
};