#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace atlas {

/**
 * View mode for the camera system.
 *
 * ORBIT  – Default EVE-style third-person orbit (RTS-style) around the ship.
 * FPS    – First-person view for walking inside ship interiors / stations.
 * COCKPIT – First-person cockpit view with flight controls visible.
 */
enum class ViewMode {
    ORBIT,
    FPS,
    COCKPIT
};

/**
 * Camera class for 3D view
 * Implements EVE-style orbit camera with smooth zoom and orbit inertia,
 * plus FPS and cockpit first-person modes for interior navigation.
 */
class Camera {
public:
    Camera(float fov = 45.0f, float aspectRatio = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 10000.0f);

    /**
     * Update camera (call each frame).
     * Applies inertia / smooth interpolation to zoom and rotation.
     */
    void update(float deltaTime);

    /**
     * Get view matrix
     */
    glm::mat4 getViewMatrix() const;

    /**
     * Get projection matrix
     */
    glm::mat4 getProjectionMatrix() const;

    /**
     * Set camera target (what to look at)
     */
    void setTarget(const glm::vec3& target);

    /**
     * Get camera position
     */
    glm::vec3 getPosition() const;

    /**
     * Camera controls
     */
    void zoom(float delta);
    void rotate(float deltaYaw, float deltaPitch);
    void pan(float deltaX, float deltaY);

    /**
     * Set aspect ratio (e.g., on window resize)
     */
    void setAspectRatio(float aspectRatio);

    /**
     * Get camera distance from target
     */
    float getDistance() const { return m_distance; }

    /**
     * Set camera distance from target
     */
    void setDistance(float distance);

    /**
     * EVE-style tracking camera: snap yaw/pitch to look at a world
     * position from the current distance.
     */
    void lookAt(const glm::vec3& worldPos);

    /**
     * Get yaw/pitch (useful for UI indicators)
     */
    float getYaw()   const { return m_yaw; }
    float getPitch() const { return m_pitch; }

    // ── View-mode API ──────────────────────────────────────────────

    /**
     * Get the current view mode.
     */
    ViewMode getViewMode() const { return m_viewMode; }

    /**
     * Set the view mode.
     * Resets inertia and snaps camera parameters to sensible defaults
     * for the new mode.
     */
    void setViewMode(ViewMode mode);

    /**
     * Set FPS / cockpit eye position and look direction.
     * Only used when view mode is FPS or COCKPIT.
     */
    void setFPSPosition(const glm::vec3& eyePos, const glm::vec3& lookDir);

    /**
     * FPS mouse-look: rotate the first-person look direction.
     * Only effective in FPS or COCKPIT mode.
     */
    void rotateFPS(float deltaYaw, float deltaPitch);

    /**
     * Get FPS forward direction (useful for movement).
     */
    glm::vec3 getFPSForward() const { return m_fpsForward; }

private:
    void updateVectors();

    // Camera parameters
    glm::vec3 m_target;
    float m_distance;
    float m_yaw;
    float m_pitch;

    // Projection parameters
    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    // Camera vectors
    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;

    // EVE-style smooth interpolation targets
    float m_targetDistance;     // desired zoom distance
    float m_yawVelocity  = 0.0f;  // angular velocity for inertia
    float m_pitchVelocity = 0.0f;

    // Current view mode
    ViewMode m_viewMode = ViewMode::ORBIT;

    // FPS / Cockpit state
    glm::vec3 m_fpsPosition{0.0f};
    glm::vec3 m_fpsForward{0.0f, 0.0f, -1.0f};
    float m_fpsYaw   = 0.0f;
    float m_fpsPitch = 0.0f;

    // Limits
    static constexpr float MIN_DISTANCE = 10.0f;
    static constexpr float MAX_DISTANCE = 5000.0f;
    static constexpr float MIN_PITCH = -89.0f;
    static constexpr float MAX_PITCH = 89.0f;

    // Smoothing parameters
    static constexpr float ZOOM_LERP_SPEED   = 8.0f;  // higher = snappier zoom
    static constexpr float INERTIA_DAMPING    = 5.0f;  // angular velocity decay rate
    static constexpr float INERTIA_THRESHOLD  = 0.05f; // stop when below this
};

} // namespace atlas
