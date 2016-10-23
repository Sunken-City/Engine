#pragma once
#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2Int.hpp"
#include "Engine/Input/InputSystem.hpp"

// Input Devices manage our raw inputs - our lowest level - tracks all hardware inputs
// (Axises, Values, what have you)
class InputDevice
{
public:
    virtual void Update(float deltaSeconds) = 0;
};

