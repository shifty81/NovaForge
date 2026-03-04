// Tests for: Warp Cinematic System Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_cinematic_system.h"

using namespace atlas;

// ==================== Warp Cinematic System Tests ====================

static void testWarpCinematicCompositeIntensityFrigate() {
    std::cout << "\n=== Warp Cinematic: Composite Intensity Frigate ===" << std::endl;
    // Frigate mass_norm=0, cruise phase_frac=0.85
    float ci = systems::WarpCinematicSystem::computeCompositeIntensity(0.0f, 0.85f);
    assertTrue(ci > 0.8f && ci <= 1.0f, "Frigate cruise composite ~ 0.85");
}

static void testWarpCinematicCompositeIntensityCapital() {
    std::cout << "\n=== Warp Cinematic: Composite Intensity Capital ===" << std::endl;
    // Capital mass_norm=1.0, cruise phase_frac=0.85
    float ci = systems::WarpCinematicSystem::computeCompositeIntensity(1.0f, 0.85f);
    assertTrue(ci > 0.85f, "Capital cruise composite > frigate");
    assertTrue(ci <= 1.0f, "Composite clamped to 1.0");
}

static void testWarpCinematicCompositeIntensityNone() {
    std::cout << "\n=== Warp Cinematic: Composite Intensity None Phase ===" << std::endl;
    float ci = systems::WarpCinematicSystem::computeCompositeIntensity(0.5f, 0.0f);
    assertTrue(approxEqual(ci, 0.0f), "No warp phase => zero composite");
}

static void testWarpCinematicLayersFrigate() {
    std::cout << "\n=== Warp Cinematic: Layers Frigate ===" << std::endl;
    float radial, bloom, skin, vignette;
    systems::WarpCinematicSystem::computeLayers(0.85f, 0.0f, radial, bloom, skin, vignette);
    assertTrue(radial > 0.0f, "Radial distortion > 0");
    assertTrue(bloom > radial, "Bloom > radial for frigate");
    assertTrue(skin > 0.0f, "Tunnel skin > 0");
    assertTrue(vignette > 0.0f, "Vignette > 0");
}

static void testWarpCinematicLayersCapitalMoreDistortion() {
    std::cout << "\n=== Warp Cinematic: Capital has more radial distortion ===" << std::endl;
    float r_frig, b_f, s_f, v_f;
    float r_cap, b_c, s_c, v_c;
    systems::WarpCinematicSystem::computeLayers(0.85f, 0.0f, r_frig, b_f, s_f, v_f);
    systems::WarpCinematicSystem::computeLayers(0.85f, 1.0f, r_cap, b_c, s_c, v_c);
    assertTrue(r_cap > r_frig, "Capital radial > frigate radial at same composite");
    assertTrue(s_c > s_f, "Capital tunnel skin > frigate at same composite");
}

static void testWarpCinematicAudioFrigate() {
    std::cout << "\n=== Warp Cinematic: Audio Frigate ===" << std::endl;
    float ev, hv, sv, ep, hp;
    systems::WarpCinematicSystem::computeAudio(0.85f, 0.0f, ev, hv, sv, ep, hp);
    assertTrue(ev > 0.0f, "Engine volume > 0");
    assertTrue(hv > 0.0f, "Harmonics volume > 0");
    assertTrue(sv > 0.0f, "Shimmer volume > 0");
    assertTrue(approxEqual(ep, 1.0f), "Frigate engine pitch = 1.0");
    assertTrue(approxEqual(hp, 1.0f), "Frigate harmonics pitch = 1.0");
}

static void testWarpCinematicAudioCapitalLowerPitch() {
    std::cout << "\n=== Warp Cinematic: Capital lower pitch ===" << std::endl;
    float ev, hv, sv, ep, hp;
    systems::WarpCinematicSystem::computeAudio(0.85f, 1.0f, ev, hv, sv, ep, hp);
    assertTrue(ep < 1.0f, "Capital engine pitch < 1.0");
    assertTrue(hp < 1.0f, "Capital harmonics pitch < 1.0");
    assertTrue(ev > 0.0f, "Capital engine volume > 0");
}

static void testWarpCinematicSystemUpdate() {
    std::cout << "\n=== Warp Cinematic: System update writes tunnel config ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->mass_norm = 0.3f;

    auto* tc = addComp<components::WarpTunnelConfig>(ship);
    auto* ap = addComp<components::WarpAudioProfile>(ship);

    sys.update(0.1f);

    assertTrue(tc->composite_intensity > 0.0f, "Composite intensity set after update");
    assertTrue(tc->radial_distortion > 0.0f, "Radial distortion set");
    assertTrue(tc->starfield_bloom > 0.0f, "Starfield bloom set");
    assertTrue(ap->engine_core_volume > 0.0f, "Engine audio volume set");
}

static void testWarpCinematicAccessibilityReducesMotion() {
    std::cout << "\n=== Warp Cinematic: Accessibility reduces motion ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->mass_norm = 0.5f;
    auto* tc = addComp<components::WarpTunnelConfig>(ship);
    auto* ap = addComp<components::WarpAudioProfile>(ship);
    auto* acc = addComp<components::WarpAccessibility>(ship);
    acc->motion_intensity = 0.5f;
    acc->bass_intensity = 0.5f;
    acc->blur_intensity = 0.5f;

    sys.update(0.1f);

    // Compare with a second entity without accessibility scaling
    auto* ship2 = world.createEntity("ship2");
    auto* ws2 = addComp<components::WarpState>(ship2);
    ws2->phase = components::WarpState::WarpPhase::Cruise;
    ws2->mass_norm = 0.5f;
    auto* tc2 = addComp<components::WarpTunnelConfig>(ship2);
    auto* ap2 = addComp<components::WarpAudioProfile>(ship2);

    sys.update(0.1f);

    assertTrue(tc->starfield_bloom < tc2->starfield_bloom, "Accessibility reduces bloom");
    assertTrue(tc->radial_distortion < tc2->radial_distortion, "Accessibility reduces distortion");
    assertTrue(ap->engine_core_volume < ap2->engine_core_volume, "Accessibility reduces bass");
}

static void testWarpCinematicNonePhaseZeroIntensity() {
    std::cout << "\n=== Warp Cinematic: None phase => zero intensity ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::None;
    ws->mass_norm = 0.5f;
    auto* tc = addComp<components::WarpTunnelConfig>(ship);
    auto* ap = addComp<components::WarpAudioProfile>(ship);

    sys.update(0.1f);

    assertTrue(approxEqual(tc->composite_intensity, 0.0f), "None phase => zero composite");
    assertTrue(approxEqual(tc->radial_distortion, 0.0f), "None phase => zero radial");
    assertTrue(approxEqual(ap->engine_core_volume, 0.0f), "None phase => zero engine audio");
}

static void testWarpCinematicAlignPhaseSubtle() {
    std::cout << "\n=== Warp Cinematic: Align phase is subtle ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::Align;
    ws->mass_norm = 0.0f;
    auto* tc = addComp<components::WarpTunnelConfig>(ship);

    sys.update(0.1f);

    assertTrue(tc->composite_intensity > 0.0f, "Align phase has some effect");
    assertTrue(tc->composite_intensity < 0.2f, "Align phase is subtle (< 0.2)");
}

static void testWarpCinematicExitPhaseFades() {
    std::cout << "\n=== Warp Cinematic: Exit phase fades out ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    // Cruise entity
    auto* s1 = world.createEntity("s1");
    auto* ws1 = addComp<components::WarpState>(s1);
    ws1->phase = components::WarpState::WarpPhase::Cruise;
    ws1->mass_norm = 0.5f;
    auto* tc1 = addComp<components::WarpTunnelConfig>(s1);

    // Exit entity
    auto* s2 = world.createEntity("s2");
    auto* ws2 = addComp<components::WarpState>(s2);
    ws2->phase = components::WarpState::WarpPhase::Exit;
    ws2->mass_norm = 0.5f;
    auto* tc2 = addComp<components::WarpTunnelConfig>(s2);

    sys.update(0.1f);

    assertTrue(tc2->composite_intensity < tc1->composite_intensity,
               "Exit phase intensity < cruise intensity");
}


void run_warp_cinematic_system_tests() {
    testWarpCinematicCompositeIntensityFrigate();
    testWarpCinematicCompositeIntensityCapital();
    testWarpCinematicCompositeIntensityNone();
    testWarpCinematicLayersFrigate();
    testWarpCinematicLayersCapitalMoreDistortion();
    testWarpCinematicAudioFrigate();
    testWarpCinematicAudioCapitalLowerPitch();
    testWarpCinematicSystemUpdate();
    testWarpCinematicAccessibilityReducesMotion();
    testWarpCinematicNonePhaseZeroIntensity();
    testWarpCinematicAlignPhaseSubtle();
    testWarpCinematicExitPhaseFades();
}
