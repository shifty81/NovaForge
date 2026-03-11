#include "rendering/camera.h"
#include <algorithm>
#include <cmath>

namespace atlas {

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : m_target(0.0f, 0.0f, 0.0f)
    , m_distance(500.0f)
    , m_yaw(0.0f)
    , m_pitch(30.0f)
    , m_fov(fov)
    , m_aspectRatio(aspectRatio)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_forward(0.0f, 0.0f, -1.0f)
    , m_right(1.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_targetDistance(500.0f)
{
    updateVectors();
}

void Camera::update(float deltaTime) {
    // ── Smooth zoom (exponential lerp toward target distance) ────────
    float distDiff = m_targetDistance - m_distance;
    if (std::abs(distDiff) > 0.01f) {
        m_distance += distDiff * std::min(1.0f, ZOOM_LERP_SPEED * deltaTime);
        m_distance = std::clamp(m_distance, MIN_DISTANCE, MAX_DISTANCE);
    } else {
        m_distance = m_targetDistance;
    }

    // ── Orbit inertia (spin continues and decays after mouse release) ─
    if (std::abs(m_yawVelocity) > INERTIA_THRESHOLD ||
        std::abs(m_pitchVelocity) > INERTIA_THRESHOLD) {
        m_yaw   += m_yawVelocity   * deltaTime;
        m_pitch += m_pitchVelocity * deltaTime;
        m_pitch  = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);

        // Exponential damping
        float decay = std::exp(-INERTIA_DAMPING * deltaTime);
        m_yawVelocity   *= decay;
        m_pitchVelocity *= decay;
    } else {
        m_yawVelocity   = 0.0f;
        m_pitchVelocity = 0.0f;
    }

    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    if (m_viewMode == ViewMode::FPS || m_viewMode == ViewMode::COCKPIT) {
        return glm::lookAt(m_fpsPosition, m_fpsPosition + m_fpsForward, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    updateVectors();
}

glm::vec3 Camera::getPosition() const {
    if (m_viewMode == ViewMode::FPS || m_viewMode == ViewMode::COCKPIT) {
        return m_fpsPosition;
    }
    return m_position;
}

void Camera::zoom(float delta) {
    // Astralis-style: scroll zooms logarithmically (proportional to current distance)
    float zoomFactor = m_targetDistance * 0.12f;
    m_targetDistance -= delta * zoomFactor;
    m_targetDistance = std::clamp(m_targetDistance, MIN_DISTANCE, MAX_DISTANCE);
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    m_yaw += deltaYaw;
    m_pitch += deltaPitch;
    
    // Clamp pitch to prevent camera flipping
    m_pitch = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);

    // Feed angular velocity for inertia when mouse is released
    // The velocity is based on the most recent deltas (weighted toward
    // responsiveness rather than averaging).
    m_yawVelocity   = deltaYaw   * 60.0f;  // scale up since deltas are per-frame
    m_pitchVelocity = deltaPitch * 60.0f;
    
    updateVectors();
}

void Camera::pan(float deltaX, float deltaY) {
    // Pan perpendicular to view direction
    float panSpeed = m_distance * 0.001f;
    m_target += m_right * deltaX * panSpeed;
    m_target += m_up * deltaY * panSpeed;
    updateVectors();
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

void Camera::setDistance(float distance) {
    m_distance = std::clamp(distance, MIN_DISTANCE, MAX_DISTANCE);
    m_targetDistance = m_distance;
    updateVectors();
}

void Camera::lookAt(const glm::vec3& worldPos) {
    glm::vec3 dir = worldPos - m_target;
    float dist = glm::length(dir);
    if (dist < 0.01f) return;

    dir = glm::normalize(dir);
    m_yaw   = glm::degrees(std::atan2(dir.x, dir.z));
    m_pitch = glm::degrees(std::asin(std::clamp(dir.y, -1.0f, 1.0f)));
    m_pitch = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);

    // Kill any lingering inertia so the snap feels intentional
    m_yawVelocity   = 0.0f;
    m_pitchVelocity = 0.0f;

    updateVectors();
}

void Camera::updateVectors() {
    // Calculate position based on spherical coordinates
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);
    
    glm::vec3 offset;
    offset.x = m_distance * cos(pitchRad) * sin(yawRad);
    offset.y = m_distance * sin(pitchRad);
    offset.z = m_distance * cos(pitchRad) * cos(yawRad);
    
    m_position = m_target + offset;
    
    // Calculate camera vectors
    m_forward = glm::normalize(m_target - m_position);
    m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}

// ── View-mode helpers ──────────────────────────────────────────────

void Camera::setViewMode(ViewMode mode) {
    if (mode == m_viewMode) return;

    // Kill orbit inertia on any mode switch
    m_yawVelocity   = 0.0f;
    m_pitchVelocity = 0.0f;

    if (mode == ViewMode::FPS || mode == ViewMode::COCKPIT) {
        // Seed FPS yaw/pitch from orbit values so the transition is seamless
        m_fpsYaw   = m_yaw;
        m_fpsPitch = m_pitch;
        // FPS position defaults to the current orbit target (ship center)
        m_fpsPosition = m_target;
        // Recompute forward from yaw/pitch (convention: yaw=0 → -Z)
        float yr = glm::radians(m_fpsYaw);
        float pr = glm::radians(m_fpsPitch);
        m_fpsForward = glm::normalize(glm::vec3(
            std::cos(pr) * std::sin(yr),
            std::sin(pr),
           -std::cos(pr) * std::cos(yr)
        ));
    }

    m_viewMode = mode;
}

void Camera::setFPSPosition(const glm::vec3& eyePos, const glm::vec3& lookDir) {
    m_fpsPosition = eyePos;
    if (glm::length(lookDir) > 0.001f) {
        m_fpsForward = glm::normalize(lookDir);
        // Derive yaw/pitch from lookDir for consistent mouse-look
        // Convention: forward = (cos(p)*sin(y), sin(p), -cos(p)*cos(y))
        // so yaw = atan2(forward.x, -forward.z)
        m_fpsYaw   = glm::degrees(std::atan2(m_fpsForward.x, -m_fpsForward.z));
        m_fpsPitch = glm::degrees(std::asin(std::clamp(m_fpsForward.y, -1.0f, 1.0f)));
    }
}

void Camera::rotateFPS(float deltaYaw, float deltaPitch) {
    m_fpsYaw   += deltaYaw;
    m_fpsPitch += deltaPitch;
    m_fpsPitch  = std::clamp(m_fpsPitch, MIN_PITCH, MAX_PITCH);

    float yr = glm::radians(m_fpsYaw);
    float pr = glm::radians(m_fpsPitch);
    m_fpsForward = glm::normalize(glm::vec3(
        std::cos(pr) * std::sin(yr),
        std::sin(pr),
       -std::cos(pr) * std::cos(yr)
    ));
}

} // namespace atlas
