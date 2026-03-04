// Tests for: WarpHUDTravelMode Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_hud_travel_mode_system.h"

using namespace atlas;

// ==================== WarpHUDTravelMode Tests ====================

static void testWarpHUDTravelModeDefaults() {
    std::cout << "\n=== WarpHUDTravelMode defaults ===" << std::endl;
    components::WarpHUDTravelMode mode;
    assertTrue(approxEqual(mode.edge_softness, 0.0f), "edge_softness default 0");
    assertTrue(approxEqual(mode.color_desaturation, 0.0f), "desaturation default 0");
    assertTrue(approxEqual(mode.warning_mute, 0.0f), "warning_mute default 0");
    assertTrue(approxEqual(mode.safe_area_padding, 0.0f), "safe_area_padding default 0");
    assertTrue(approxEqual(mode.hud_scale, 1.0f), "hud_scale default 1");
    assertTrue(!mode.ui_flair_enabled, "ui_flair disabled by default");
    assertTrue(approxEqual(mode.bracket_animation, 0.0f), "bracket_animation default 0");
    assertTrue(approxEqual(mode.ui_glow_intensity, 0.0f), "ui_glow default 0");
    assertTrue(approxEqual(mode.hud_parallax_offset, 0.0f), "parallax default 0");
}

static void testHUDTargetsNonePhase() {
    std::cout << "\n=== HUD targets: None phase ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(0, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.0f), "edge_softness 0 for None");
    assertTrue(approxEqual(d, 0.0f), "desaturation 0 for None");
    assertTrue(approxEqual(w, 0.0f), "warning_mute 0 for None");
    assertTrue(approxEqual(p, 0.0f), "padding 0 for None");
    assertTrue(approxEqual(s, 1.0f), "scale 1.0 for None");
}

static void testHUDTargetsCruisePhase() {
    std::cout << "\n=== HUD targets: Cruise phase ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(3, e, d, w, p, s);
    assertTrue(e > 0.5f, "cruise edge_softness high");
    assertTrue(d > 0.2f, "cruise desaturation noticeable");
    assertTrue(w > 0.5f, "cruise warnings muted");
    assertTrue(p >= 32.0f && p <= 48.0f, "cruise padding 32-48px");
    assertTrue(s < 1.0f && s >= 0.95f, "cruise scale inward");
}

static void testHUDTargetsExitPhase() {
    std::cout << "\n=== HUD targets: Exit phase ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(5, e, d, w, p, s);
    assertTrue(e < 0.3f, "exit edge_softness reducing");
    assertTrue(d < 0.15f, "exit desaturation low");
    assertTrue(approxEqual(w, 0.0f), "exit warnings not muted");
    assertTrue(p < 16.0f, "exit padding almost gone");
    assertTrue(s > 0.97f, "exit scale nearly normal");
}

static void testHUDTargetsEventPhase() {
    std::cout << "\n=== HUD targets: Event phase ===" << std::endl;
    float e_event, d_event, w_event, p_event, s_event;
    systems::WarpHUDTravelModeSystem::computeTargets(4, e_event, d_event, w_event, p_event, s_event);
    float e_cruise, d_cruise, w_cruise, p_cruise, s_cruise;
    systems::WarpHUDTravelModeSystem::computeTargets(3, e_cruise, d_cruise, w_cruise, p_cruise, s_cruise);
    assertTrue(w_event < w_cruise, "event warning_mute less than cruise");
}

static void testUIFlairComputation() {
    std::cout << "\n=== UI flair computation ===" << std::endl;
    float bracket, glow, parallax;
    systems::WarpHUDTravelModeSystem::computeUIFlair(0.0f, 0.5f, bracket, glow, parallax);
    assertTrue(approxEqual(bracket, 0.5f, 0.02f), "bracket ~0.5 at time 0");
    assertTrue(glow > 0.0f && glow <= 0.5f, "glow tracks bass (0.5)");
    assertTrue(std::fabs(parallax) < 0.1f, "parallax ~0 at time 0");
}

static void testUIFlairGlowTracksBass() {
    std::cout << "\n=== UI flair glow tracks bass ===" << std::endl;
    float bracket, glow, parallax;
    systems::WarpHUDTravelModeSystem::computeUIFlair(5.0f, 0.0f, bracket, glow, parallax);
    assertTrue(approxEqual(glow, 0.0f), "no bass = no glow");
    systems::WarpHUDTravelModeSystem::computeUIFlair(5.0f, 1.0f, bracket, glow, parallax);
    assertTrue(glow > 0.7f, "full bass = high glow");
}

static void testHUDSystemRampsDuringCruise() {
    std::cout << "\n=== HUD system ramps during cruise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_warp_hud_cruise");
    auto* ws = addComp<components::WarpState>(e);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->warp_time = 10.0f;
    addComp<components::WarpHUDTravelMode>(e);

    systems::WarpHUDTravelModeSystem sys(&world);
    for (int i = 0; i < 60; ++i) sys.update(0.05f);

    auto* hud = e->getComponent<components::WarpHUDTravelMode>();
    assertTrue(hud->edge_softness > 0.4f, "edge_softness ramped up");
    assertTrue(hud->color_desaturation > 0.2f, "desaturation ramped up");
    assertTrue(hud->safe_area_padding > 30.0f, "padding ramped up");
    assertTrue(hud->hud_scale < 0.98f, "scale ramped down");
}

static void testHUDSystemReturnsToNormalOnNone() {
    std::cout << "\n=== HUD system returns to normal on None ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_warp_hud_return");
    auto* ws = addComp<components::WarpState>(e);
    addComp<components::WarpHUDTravelMode>(e);

    systems::WarpHUDTravelModeSystem sys(&world);

    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->warp_time = 10.0f;
    for (int i = 0; i < 60; ++i) sys.update(0.05f);

    ws->phase = components::WarpState::WarpPhase::None;
    for (int i = 0; i < 80; ++i) sys.update(0.05f);

    auto* hud = e->getComponent<components::WarpHUDTravelMode>();
    assertTrue(hud->edge_softness < 0.05f, "edge_softness back to ~0");
    assertTrue(hud->color_desaturation < 0.05f, "desaturation back to ~0");
    assertTrue(hud->safe_area_padding < 2.0f, "padding back to ~0");
    assertTrue(hud->hud_scale > 0.99f, "scale back to ~1.0");
}

static void testHUDSystemUIFlairOnlyDuringWarp() {
    std::cout << "\n=== HUD UI flair only during warp ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_warp_hud_flair");
    auto* ws = addComp<components::WarpState>(e);
    ws->phase = components::WarpState::WarpPhase::None;
    ws->warp_time = 5.0f;
    auto* hud = addComp<components::WarpHUDTravelMode>(e);
    hud->ui_flair_enabled = true;

    systems::WarpHUDTravelModeSystem sys(&world);
    sys.update(0.016f);

    assertTrue(approxEqual(hud->bracket_animation, 0.0f), "no flair when not in warp");
    assertTrue(approxEqual(hud->ui_glow_intensity, 0.0f), "no glow when not in warp");

    ws->phase = components::WarpState::WarpPhase::Cruise;
    auto* audio = addComp<components::WarpAudioProfile>(e);
    audio->engine_core_volume = 0.6f;
    sys.update(0.016f);

    assertTrue(hud->bracket_animation > 0.0f || hud->ui_glow_intensity > 0.0f,
               "flair active during cruise");
}


void run_warp_hudtravel_mode_tests() {
    testWarpHUDTravelModeDefaults();
    testHUDTargetsNonePhase();
    testHUDTargetsCruisePhase();
    testHUDTargetsExitPhase();
    testHUDTargetsEventPhase();
    testUIFlairComputation();
    testUIFlairGlowTracksBass();
    testHUDSystemRampsDuringCruise();
    testHUDSystemReturnsToNormalOnNone();
    testHUDSystemUIFlairOnlyDuringWarp();
}
