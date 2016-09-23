#pragma once

// Input Devices manage our raw inputs - our lowest level - tracks all hardware inputs
// (Axises, Values, what have you)
class InputDevice
{
public:
    virtual void Update() = 0;
};