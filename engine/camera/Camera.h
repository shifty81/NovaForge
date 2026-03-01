#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace atlas::camera {

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 Normalized() const {
        float len = Length();
        if (len < 1e-8f) return {0, 0, 0};
        return {x / len, y / len, z / len};
    }
};

enum class CameraMode : uint8_t {
    FreeLook,
    Strategy,
    FPS,
    Orbital
};

class Camera {
public:
    void SetMode(CameraMode mode);
    CameraMode GetMode() const;

    void SetPosition(float x, float y, float z);
    Vec3 GetPosition() const;

    void SetTarget(float x, float y, float z);
    Vec3 GetTarget() const;

    void SetFOV(float fovDegrees);
    float GetFOV() const;

    void SetClipPlanes(float nearPlane, float farPlane);
    float GetNearPlane() const;
    float GetFarPlane() const;

    void SetOrbitalRadius(float radius);
    float GetOrbitalRadius() const;

    void SetYawPitch(float yaw, float pitch);
    float GetYaw() const;
    float GetPitch() const;

    void MoveForward(float amount);
    void MoveRight(float amount);
    void MoveUp(float amount);
    void Orbit(float deltaYaw, float deltaPitch);

    void Update(float dt);

    Vec3 GetForward() const;
    Vec3 GetRight() const;

private:
    CameraMode m_mode = CameraMode::FreeLook;
    Vec3 m_position = {0, 0, 5};
    Vec3 m_target = {0, 0, 0};
    float m_fov = 60.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
    float m_orbitalRadius = 10.0f;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_moveSpeed = 5.0f;
};

}
