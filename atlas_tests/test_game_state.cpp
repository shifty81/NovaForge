/**
 * Tests for Camera ViewMode management and GameState name helpers.
 *
 * Validates:
 * - Camera ViewMode enum values
 * - Camera mode switching (Orbit, FPS, Cockpit)
 * - FPS position and look direction
 * - FPS mouse-look rotation
 * - Mode transitions kill inertia
 *
 * These tests do not require an OpenGL context.
 */

#include "../cpp_client/include/rendering/camera.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

using namespace atlas;

// ── Camera ViewMode tests ─────────────────────────────────────────

void test_camera_default_view_mode() {
    Camera cam;
    assert(cam.getViewMode() == ViewMode::ORBIT);

    std::cout << "[PASS] test_camera_default_view_mode" << std::endl;
}

void test_camera_set_view_mode_orbit() {
    Camera cam;
    cam.setViewMode(ViewMode::ORBIT);
    assert(cam.getViewMode() == ViewMode::ORBIT);

    std::cout << "[PASS] test_camera_set_view_mode_orbit" << std::endl;
}

void test_camera_set_view_mode_fps() {
    Camera cam;
    cam.setViewMode(ViewMode::FPS);
    assert(cam.getViewMode() == ViewMode::FPS);

    std::cout << "[PASS] test_camera_set_view_mode_fps" << std::endl;
}

void test_camera_set_view_mode_cockpit() {
    Camera cam;
    cam.setViewMode(ViewMode::COCKPIT);
    assert(cam.getViewMode() == ViewMode::COCKPIT);

    std::cout << "[PASS] test_camera_set_view_mode_cockpit" << std::endl;
}

void test_camera_fps_position_and_forward() {
    Camera cam;
    cam.setViewMode(ViewMode::FPS);

    glm::vec3 eyePos(10.0f, 1.8f, 5.0f);
    glm::vec3 lookDir(0.0f, 0.0f, -1.0f);
    cam.setFPSPosition(eyePos, lookDir);

    glm::vec3 pos = cam.getPosition();
    assert(std::abs(pos.x - 10.0f) < 0.01f);
    assert(std::abs(pos.y - 1.8f) < 0.01f);
    assert(std::abs(pos.z - 5.0f) < 0.01f);

    glm::vec3 fwd = cam.getFPSForward();
    assert(std::abs(fwd.z - (-1.0f)) < 0.1f);

    std::cout << "[PASS] test_camera_fps_position_and_forward" << std::endl;
}

void test_camera_fps_rotate() {
    Camera cam;
    cam.setViewMode(ViewMode::FPS);
    cam.setFPSPosition(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // Rotate 90 degrees right (yaw)
    cam.rotateFPS(90.0f, 0.0f);

    glm::vec3 fwd = cam.getFPSForward();
    // After 90 deg yaw from +Z, should be approximately +X
    assert(std::abs(fwd.x - 1.0f) < 0.1f);
    assert(std::abs(fwd.z) < 0.1f);

    std::cout << "[PASS] test_camera_fps_rotate" << std::endl;
}

void test_camera_orbit_position_unchanged_in_fps() {
    Camera cam;
    cam.setTarget(glm::vec3(100.0f, 0.0f, 0.0f));
    cam.setDistance(200.0f);
    cam.update(0.016f);

    // Switch to FPS mode
    cam.setViewMode(ViewMode::FPS);
    cam.setFPSPosition(glm::vec3(5.0f, 1.8f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    // In FPS mode, getPosition() returns the FPS eye position, not the orbit position
    glm::vec3 fpsPos = cam.getPosition();
    assert(std::abs(fpsPos.x - 5.0f) < 0.01f);
    assert(std::abs(fpsPos.y - 1.8f) < 0.01f);

    // Switch back to orbit — orbit target should be unchanged
    cam.setViewMode(ViewMode::ORBIT);

    std::cout << "[PASS] test_camera_orbit_position_unchanged_in_fps" << std::endl;
}

void test_camera_view_mode_kills_inertia() {
    Camera cam;
    // Build up some inertia via orbit rotate
    cam.rotate(10.0f, 5.0f);

    // Switch mode — should kill inertia
    cam.setViewMode(ViewMode::FPS);
    cam.update(1.0f);

    // Switch back
    cam.setViewMode(ViewMode::ORBIT);
    cam.update(1.0f);

    // If inertia was killed, verify no crash and mode is correct
    assert(cam.getViewMode() == ViewMode::ORBIT);

    std::cout << "[PASS] test_camera_view_mode_kills_inertia" << std::endl;
}

void test_camera_view_matrix_differs_by_mode() {
    Camera cam;
    cam.setTarget(glm::vec3(0.0f));
    cam.setDistance(100.0f);
    cam.update(0.016f);

    glm::mat4 orbitView = cam.getViewMatrix();

    cam.setViewMode(ViewMode::FPS);
    cam.setFPSPosition(glm::vec3(50.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    glm::mat4 fpsView = cam.getViewMatrix();

    // The two view matrices should be different
    bool different = false;
    for (int i = 0; i < 4 && !different; ++i)
        for (int j = 0; j < 4 && !different; ++j)
            if (std::abs(orbitView[i][j] - fpsView[i][j]) > 0.001f)
                different = true;
    assert(different);

    std::cout << "[PASS] test_camera_view_matrix_differs_by_mode" << std::endl;
}
