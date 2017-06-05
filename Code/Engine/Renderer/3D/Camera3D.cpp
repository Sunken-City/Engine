#include "Engine/Renderer/3D/Camera3D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/InputDevices/MouseInputDevice.hpp"

//-----------------------------------------------------------------------------------
Camera3D::Camera3D()
: m_orientation(0.0f, 0.0f, 0.0f)
, m_position(0.0f, 0.0f, 0.0f)
{
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetForward() const
{
    float cosYaw = cos(m_orientation.GetYawRadiansAboutZ());
    float sinYaw = sin(m_orientation.GetYawRadiansAboutZ());
    float cosPitch = cos(m_orientation.GetPitchRadiansAboutY());
    float sinPitch = sin(m_orientation.GetPitchRadiansAboutY());
    return Vector3(cosYaw * cosPitch, -sinPitch, sinYaw * cosPitch);
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetForwardTwoComponent() const
{
    float cosYaw = cos(m_orientation.GetYawRadiansAboutZ());
    float sinYaw = sin(m_orientation.GetYawRadiansAboutZ());
    return (Vector3::FORWARD * cosYaw) + (Vector3::RIGHT * -sinYaw);
}

//-----------------------------------------------------------------------------------
Vector3 Camera3D::GetLeft() const
{
    float cosYaw = cos(m_orientation.GetYawRadiansAboutZ());
    float sinYaw = sin(m_orientation.GetYawRadiansAboutZ());
    return (Vector3::RIGHT * -cosYaw) + (Vector3::FORWARD * -sinYaw);
}

//-----------------------------------------------------------------------------------
Matrix4x4 Camera3D::GetViewMatrix()
{
    //Set up view from camera
    Matrix4x4 view;
    Matrix4x4::MatrixMakeIdentity(&view);
    //Negative on the yaw prevents left-right motion from being inverted.
    Matrix4x4::MatrixMakeRotationEuler(&view, -m_orientation.GetYawRadiansAboutZ(), m_orientation.GetPitchRadiansAboutY(), m_orientation.GetRollRadiansAboutX(), m_position);
    Matrix4x4::MatrixInvertOrthogonal(&view);
    return view;
}

//-----------------------------------------------------------------------------------
void Camera3D::LookAt(const Vector3& position)
{
    Matrix4x4 lookAtMat;
    Matrix4x4::MatrixMakeLookAt(&lookAtMat, m_position, position, Vector3::UP);
    m_orientation = lookAtMat.GetEulerRotation();
}

//-----------------------------------------------------------------------------------
void Camera3D::Render()
{

}

//-----------------------------------------------------------------------------------
void Camera3D::UpdateFromInput(float deltaTime)
{
    float moveSpeed;

    if (InputSystem::instance->IsKeyDown(InputSystem::ExtraKeys::SHIFT))
    {
        moveSpeed = BASE_MOVE_SPEED * 8.0f;
    }
    else
    {
        moveSpeed = BASE_MOVE_SPEED;
    }
    if (InputSystem::instance->IsKeyDown('W'))
    {
        Vector3 cameraForwardXY = GetForwardTwoComponent();
        m_position += cameraForwardXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('S'))
    {
        Vector3 cameraForwardXY = GetForwardTwoComponent();
        m_position -= cameraForwardXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('D'))
    {
        Vector3 cameraLeftXY = GetLeft();
        m_position -= cameraLeftXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('A'))
    {
        Vector3 camreaLeftXY = GetLeft();
        m_position += camreaLeftXY * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown(' '))
    {
        m_position += Vector3::UP * (moveSpeed * deltaTime);
    }
    if (InputSystem::instance->IsKeyDown('Z'))
    {
        m_position -= Vector3::UP * (moveSpeed * deltaTime);
    }

    MouseInputDevice::CaptureMouseCursor();
    Vector2Int cursorDelta = InputSystem::instance->GetDeltaMouse();

    //Prevents pitch from going above 89.9
    m_orientation.yawDegreesAboutZ -= ((float)cursorDelta.x * MOUSE_ROTATION_SPEED);
    float proposedPitch = m_orientation.pitchDegreesAboutY - ((float)cursorDelta.y * MOUSE_ROTATION_SPEED);
    m_orientation.pitchDegreesAboutY = MathUtils::Clamp(proposedPitch, -CAMERA_PITCH_ANGLE_CLAMP_DEGREES, CAMERA_PITCH_ANGLE_CLAMP_DEGREES);
}

//-----------------------------------------------------------------------------------
void Camera3D::Update(float deltaTime)
{
    if (m_updateFromInput)
    {
        UpdateFromInput(deltaTime);
    }
}