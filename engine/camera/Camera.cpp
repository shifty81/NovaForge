#include "Camera.h"

namespace atlas::camera {

void Camera::SetMode(CameraMode mode) {
    m_mode = mode;
}

CameraMode Camera::GetMode() const {
    return m_mode;
}

void Camera::SetPosition(float x, float y, float z) {
    m_position = {x, y, z};
}

Vec3 Camera::GetPosition() const {
    return m_position;
}

void Camera::SetTarget(float x, float y, float z) {
    m_target = {x, y, z};
}

Vec3 Camera::GetTarget() const {
    return m_target;
}

void Camera::SetFOV(float fovDegrees) {
    m_fov = std::clamp(fovDegrees, 1.0f, 179.0f);
}

float Camera::GetFOV() const {
    return m_fov;
}

void Camera::SetClipPlanes(float nearPlane, float farPlane) {
    m_nearPlane = nearPlane > 0.0f ? nearPlane : 0.01f;
    m_farPlane = farPlane > m_nearPlane ? farPlane : m_nearPlane + 1.0f;
}

float Camera::GetNearPlane() const {
    return m_nearPlane;
}

float Camera::GetFarPlane() const {
    return m_farPlane;
}

void Camera::SetOrbitalRadius(float radius) {
    m_orbitalRadius = radius > 0.1f ? radius : 0.1f;
}

float Camera::GetOrbitalRadius() const {
    return m_orbitalRadius;
}

void Camera::SetYawPitch(float yaw, float pitch) {
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, -89.0f, 89.0f);
}

float Camera::GetYaw() const {
    return m_yaw;
}

float Camera::GetPitch() const {
    return m_pitch;
}

Vec3 Camera::GetForward() const {
    float yawRad = m_yaw * 3.14159265f / 180.0f;
    float pitchRad = m_pitch * 3.14159265f / 180.0f;
    return Vec3{
        std::cos(pitchRad) * std::sin(yawRad),
        std::sin(pitchRad),
        std::cos(pitchRad) * std::cos(yawRad)
    }.Normalized();
}

Vec3 Camera::GetRight() const {
    float yawRad = m_yaw * 3.14159265f / 180.0f;
    return Vec3{
        std::cos(yawRad),
        0.0f,
        -std::sin(yawRad)
    }.Normalized();
}

void Camera::MoveForward(float amount) {
    Vec3 fwd = GetForward();
    m_position = m_position + fwd * (amount * m_moveSpeed);
}

void Camera::MoveRight(float amount) {
    Vec3 right = GetRight();
    m_position = m_position + right * (amount * m_moveSpeed);
}

void Camera::MoveUp(float amount) {
    m_position.y += amount * m_moveSpeed;
}

void Camera::Orbit(float deltaYaw, float deltaPitch) {
    m_yaw += deltaYaw;
    m_pitch = std::clamp(m_pitch + deltaPitch, -89.0f, 89.0f);

    if (m_mode == CameraMode::Orbital) {
        float yawRad = m_yaw * 3.14159265f / 180.0f;
        float pitchRad = m_pitch * 3.14159265f / 180.0f;
        m_position.x = m_target.x + m_orbitalRadius * std::cos(pitchRad) * std::sin(yawRad);
        m_position.y = m_target.y + m_orbitalRadius * std::sin(pitchRad);
        m_position.z = m_target.z + m_orbitalRadius * std::cos(pitchRad) * std::cos(yawRad);
    }
}

void Camera::Update(float dt) {
    (void)dt;
    if (m_mode == CameraMode::Strategy) {
        m_position.y = std::max(m_position.y, 1.0f);
    }
}

}
