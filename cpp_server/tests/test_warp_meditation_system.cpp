// Tests for: Warp Meditation System Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_meditation_system.h"

using namespace atlas;

// ==================== Warp Meditation System Tests ====================

static void testWarpMeditationDefaults() {
    std::cout << "\n=== Warp Meditation Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med1");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    assertTrue(!med->active, "Meditation not active by default");
    assertTrue(approxEqual(med->volume, 0.0f), "Volume is 0 by default");
    assertTrue(approxEqual(med->activation_delay, 15.0f), "Activation delay default 15s");
    assertTrue(approxEqual(med->fade_duration, 5.0f), "Fade duration default 5s");
}

static void testMeditationActivatesAfterDelay() {
    std::cout << "\n=== Meditation Activates After Delay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med2");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 10.0f;

    systems::WarpMeditationSystem sys(&world);
    // Not enough time yet
    sys.update(5.0f);
    assertTrue(!med->active, "Not active before delay");
    // Enough time
    sys.update(6.0f);
    assertTrue(med->active, "Active after delay reached");
}

static void testMeditationFadesInDuringCruise() {
    std::cout << "\n=== Meditation Fades In During Cruise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med3");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 0.0f;
    med->fade_duration = 10.0f;

    systems::WarpMeditationSystem sys(&world);
    sys.update(1.0f);
    assertTrue(med->active, "Active after activation");
    sys.update(5.0f);
    assertTrue(med->volume > 0.0f && med->volume < 1.0f, "Volume ramping up");
    sys.update(10.0f);
    assertTrue(approxEqual(med->volume, 1.0f), "Volume reaches 1.0");
}

static void testMeditationFadesOutOnExit() {
    std::cout << "\n=== Meditation Fades Out On Exit ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med4");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 0.0f;
    med->fade_duration = 5.0f;

    systems::WarpMeditationSystem sys(&world);
    sys.update(1.0f);
    sys.update(10.0f); // fully faded in
    assertTrue(approxEqual(med->volume, 1.0f), "Volume at 1.0");

    warp->phase = components::WarpState::WarpPhase::None;
    sys.update(2.0f);
    assertTrue(med->volume < 1.0f, "Volume decreasing after exit");
}

static void testMeditationResetOnNone() {
    std::cout << "\n=== Meditation Reset On None ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med5");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 5.0f;

    systems::WarpMeditationSystem sys(&world);
    sys.update(6.0f);
    assertTrue(med->warp_cruise_time > 0.0f, "Cruise time accumulated");

    warp->phase = components::WarpState::WarpPhase::None;
    sys.update(10.0f);
    assertTrue(approxEqual(med->warp_cruise_time, 0.0f), "Cruise time reset to 0");
}

static void testAudioProgressionPhases() {
    std::cout << "\n=== Audio Progression Phases ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("prog1");
    auto* prog = addComp<components::WarpAudioProgression>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    prog->tension_duration = 3.0f;
    prog->stabilize_duration = 5.0f;
    prog->bloom_duration = 4.0f;

    systems::WarpMeditationSystem sys(&world);
    using Phase = components::WarpAudioProgression::Phase;

    sys.update(1.0f);
    assertTrue(prog->current_phase == Phase::Tension, "Phase is Tension at 1s");
    sys.update(3.0f); // total 4s
    assertTrue(prog->current_phase == Phase::Stabilize, "Phase is Stabilize at 4s");
    sys.update(5.0f); // total 9s
    assertTrue(prog->current_phase == Phase::Bloom, "Phase is Bloom at 9s");
    sys.update(4.0f); // total 13s
    assertTrue(prog->current_phase == Phase::Meditative, "Phase is Meditative at 13s");
}

static void testAudioProgressionReset() {
    std::cout << "\n=== Audio Progression Reset ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("prog2");
    auto* prog = addComp<components::WarpAudioProgression>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;

    systems::WarpMeditationSystem sys(&world);
    sys.update(20.0f);
    using Phase = components::WarpAudioProgression::Phase;
    assertTrue(prog->current_phase == Phase::Meditative, "Reached Meditative");

    warp->phase = components::WarpState::WarpPhase::None;
    sys.update(0.1f);
    assertTrue(prog->current_phase == Phase::Tension, "Reset to Tension on None");
    assertTrue(approxEqual(prog->phase_timer, 0.0f), "Timer reset to 0");
}

static void testAudioProgressionComputeOverall() {
    std::cout << "\n=== Audio Progression Compute Overall ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("prog3");
    auto* prog = addComp<components::WarpAudioProgression>(e);
    prog->tension_duration = 3.0f;
    prog->stabilize_duration = 5.0f;
    prog->bloom_duration = 4.0f;

    prog->phase_timer = 0.0f;
    assertTrue(approxEqual(prog->computeOverallProgression(), 0.0f), "Overall 0 at start");

    prog->phase_timer = 6.0f; // halfway through total 12s
    assertTrue(approxEqual(prog->computeOverallProgression(), 0.5f), "Overall 0.5 at midpoint");

    prog->phase_timer = 12.0f;
    assertTrue(approxEqual(prog->computeOverallProgression(), 1.0f), "Overall 1.0 at end");

    prog->phase_timer = 20.0f; // past end
    assertTrue(approxEqual(prog->computeOverallProgression(), 1.0f), "Overall clamped to 1.0");
}


void run_warp_meditation_system_tests() {
    testWarpMeditationDefaults();
    testMeditationActivatesAfterDelay();
    testMeditationFadesInDuringCruise();
    testMeditationFadesOutOnExit();
    testMeditationResetOnNone();
    testAudioProgressionPhases();
    testAudioProgressionReset();
    testAudioProgressionComputeOverall();
}
