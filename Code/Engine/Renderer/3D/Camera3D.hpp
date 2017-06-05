#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Matrix4x4.hpp"

class Camera3D
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Camera3D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaTime);
    void Render();
    void UpdateFromInput(float deltaTime);
    Vector3 GetForward() const;
    Vector3 GetForwardTwoComponent() const;
    Vector3 GetLeft() const;
    Matrix4x4 GetViewMatrix();
    void LookAt(const Vector3& position);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static constexpr float BASE_MOVE_SPEED = 4.5f;
    static constexpr float CAMERA_PITCH_ANGLE_CLAMP_DEGREES = 90.0f;
    static constexpr float MOUSE_ROTATION_SPEED = 0.05f;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Vector3 m_position;
    EulerAngles m_orientation;
    bool m_updateFromInput = false;
};
