// Tests for: WarpHUDTravelModeSystem Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_hud_travel_mode_system.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace atlas;

// ==================== WarpHUDTravelModeSystem Tests ====================

static void testWarpHUDPhaseNone() {
    std::cout << "\n=== WarpHUDSystem: Phase None ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(0, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.0f), "None: edge_softness = 0");
    assertTrue(approxEqual(d, 0.0f), "None: desaturation = 0");
    assertTrue(approxEqual(w, 0.0f), "None: warning_mute = 0");
    assertTrue(approxEqual(p, 0.0f), "None: safe_area_padding = 0");
    assertTrue(approxEqual(s, 1.0f), "None: hud_scale = 1.0");
}

static void testWarpHUDPhaseAlign() {
    std::cout << "\n=== WarpHUDSystem: Phase Align ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(1, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.1f), "Align: edge_softness = 0.1");
    assertTrue(approxEqual(d, 0.05f), "Align: desaturation = 0.05");
    assertTrue(approxEqual(w, 0.0f), "Align: warning_mute = 0.0");
    assertTrue(approxEqual(p, 8.0f), "Align: safe_area_padding = 8");
    assertTrue(approxEqual(s, 0.99f), "Align: hud_scale = 0.99");
}

static void testWarpHUDPhaseCruise() {
    std::cout << "\n=== WarpHUDSystem: Phase Cruise ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(3, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.6f), "Cruise: edge_softness = 0.6");
    assertTrue(approxEqual(d, 0.3f), "Cruise: desaturation = 0.3");
    assertTrue(approxEqual(w, 0.7f), "Cruise: warning_mute = 0.7");
    assertTrue(approxEqual(p, 40.0f), "Cruise: safe_area_padding = 40");
    assertTrue(approxEqual(s, 0.95f), "Cruise: hud_scale = 0.95");
}

static void testWarpHUDAllPhases() {
    std::cout << "\n=== WarpHUDSystem: All Phases ===" << std::endl;
    float e, d, w, p, s;

    // Phase 2 (Entry)
    systems::WarpHUDTravelModeSystem::computeTargets(2, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.35f), "Entry: edge_softness = 0.35");
    assertTrue(approxEqual(d, 0.15f), "Entry: desaturation = 0.15");
    assertTrue(approxEqual(w, 0.3f), "Entry: warning_mute = 0.3");
    assertTrue(approxEqual(p, 24.0f), "Entry: safe_area_padding = 24");
    assertTrue(approxEqual(s, 0.97f), "Entry: hud_scale = 0.97");

    // Phase 4 (Event)
    systems::WarpHUDTravelModeSystem::computeTargets(4, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.4f), "Event: edge_softness = 0.4");
    assertTrue(approxEqual(d, 0.2f), "Event: desaturation = 0.2");
    assertTrue(approxEqual(w, 0.3f), "Event: warning_mute = 0.3");
    assertTrue(approxEqual(p, 32.0f), "Event: safe_area_padding = 32");
    assertTrue(approxEqual(s, 0.96f), "Event: hud_scale = 0.96");

    // Phase 5 (Exit)
    systems::WarpHUDTravelModeSystem::computeTargets(5, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.15f), "Exit: edge_softness = 0.15");
    assertTrue(approxEqual(d, 0.05f), "Exit: desaturation = 0.05");
    assertTrue(approxEqual(w, 0.0f), "Exit: warning_mute = 0.0");
    assertTrue(approxEqual(p, 10.0f), "Exit: safe_area_padding = 10");
    assertTrue(approxEqual(s, 0.99f), "Exit: hud_scale = 0.99");
}

static void testWarpHUDUIFlairAtZero() {
    std::cout << "\n=== WarpHUDSystem: UI Flair at Zero ===" << std::endl;
    float bracket, glow, parallax;
    systems::WarpHUDTravelModeSystem::computeUIFlair(0.0f, 0.0f, bracket, glow, parallax);

    float expected_bracket = 0.5f + 0.5f * std::sin(0.0f);
    assertTrue(approxEqual(bracket, expected_bracket), "bracket_anim = 0.5 at t=0, bass=0");
    assertTrue(approxEqual(glow, 0.0f), "glow = 0 with bass=0");
    float expected_parallax = 3.0f * std::sin(0.0f);
    assertTrue(approxEqual(parallax, expected_parallax), "parallax = 0 at t=0");
}

static void testWarpHUDUIFlairNonZero() {
    std::cout << "\n=== WarpHUDSystem: UI Flair Non-Zero ===" << std::endl;
    float bracket, glow, parallax;

    // Test at time=5.0, bass=0.5
    systems::WarpHUDTravelModeSystem::computeUIFlair(5.0f, 0.5f, bracket, glow, parallax);
    float expected_bracket = 0.5f + 0.5f * std::sin(5.0f * 0.15f * 2.0f * M_PI);
    assertTrue(approxEqual(bracket, expected_bracket, 0.02f), "bracket matches formula at t=5");
    float expected_glow = std::fmin(std::fmax(0.5f * 0.8f, 0.0f), 1.0f);
    assertTrue(approxEqual(glow, expected_glow), "glow = clamp(0.5*0.8, 0, 1) = 0.4");
    float expected_parallax = 3.0f * std::sin(5.0f * 0.1f * 2.0f * M_PI);
    assertTrue(approxEqual(parallax, expected_parallax, 0.02f), "parallax matches formula at t=5");

    // Test at time=10.0, bass=1.0
    systems::WarpHUDTravelModeSystem::computeUIFlair(10.0f, 1.0f, bracket, glow, parallax);
    float expected_glow_full = std::fmin(1.0f * 0.8f, 1.0f);
    assertTrue(approxEqual(glow, expected_glow_full), "glow = 0.8 with bass=1.0");

    // Test glow clamp with bass > 1.25 (would exceed 1.0)
    systems::WarpHUDTravelModeSystem::computeUIFlair(0.0f, 1.5f, bracket, glow, parallax);
    assertTrue(glow <= 1.0f, "glow clamped to 1.0 max");
}

static void testWarpHUDRampConstant() {
    std::cout << "\n=== WarpHUDSystem: Ramp Speed Constant ===" << std::endl;
    assertTrue(approxEqual(systems::WarpHUDTravelModeSystem::kDefaultRampSpeed, 1.5f),
               "kDefaultRampSpeed == 1.5f");
}


void run_warp_hud_travel_mode_system_tests() {
    testWarpHUDPhaseNone();
    testWarpHUDPhaseAlign();
    testWarpHUDPhaseCruise();
    testWarpHUDAllPhases();
    testWarpHUDUIFlairAtZero();
    testWarpHUDUIFlairNonZero();
    testWarpHUDRampConstant();
}
