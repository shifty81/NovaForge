// Tests for: FPSStealthSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_stealth_system.h"

using namespace atlas;

// ==================== FPSStealthSystem Tests ====================

static void testStealthInitialState() {
    std::cout << "\n=== FPSStealth: Initial State ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::FPSStealth>(e);

    assertTrue(sys.getDetectionState("player_1") == 0, "Hidden initially");
    assertTrue(approxEqual(sys.getDetectionMeter("player_1"), 0.0f), "Detection meter at 0");
}

static void testStealthDetectionMeter() {
    std::cout << "\n=== FPSStealth: Detection Meter ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->suspicious_threshold = 0.3f;
    stealth->detected_threshold = 0.7f;
    stealth->detection_decay_rate = 0.0f;  // No decay for this test

    sys.update(0.1f);  // Init
    assertTrue(sys.addDetection("player_1", 0.2f), "Add 0.2 detection");
    sys.update(0.1f);
    assertTrue(sys.getDetectionState("player_1") == 0, "Still Hidden at 0.2");

    sys.addDetection("player_1", 0.2f);  // Now 0.4
    sys.update(0.1f);
    assertTrue(sys.getDetectionState("player_1") == 1, "Suspicious at 0.4");

    sys.addDetection("player_1", 0.4f);  // Now 0.8
    sys.update(0.1f);
    assertTrue(sys.getDetectionState("player_1") == 2, "Detected at 0.8");

    sys.addDetection("player_1", 0.3f);  // Now 1.0 (capped)
    sys.update(0.1f);
    assertTrue(sys.getDetectionState("player_1") == 3, "FullAlert at 1.0");
}

static void testStealthDecay() {
    std::cout << "\n=== FPSStealth: Detection Decay ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->detection_decay_rate = 1.0f;  // Fast decay

    sys.addDetection("player_1", 0.8f);
    sys.update(0.1f);
    assertTrue(sys.getDetectionState("player_1") == 2, "Detected");

    sys.update(1.0f);  // 1s of decay at 1.0/s = lose 1.0
    assertTrue(sys.getDetectionState("player_1") == 0, "Back to Hidden after decay");
}

static void testStealthCrouch() {
    std::cout << "\n=== FPSStealth: Crouch Visibility ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->light_level = 1.0f;
    stealth->detection_decay_rate = 0.0f;

    sys.update(0.1f);
    float vis_standing = sys.getVisibility("player_1");

    sys.setCrouching("player_1", true);
    sys.update(0.1f);
    float vis_crouching = sys.getVisibility("player_1");

    assertTrue(vis_crouching < vis_standing, "Crouching reduces visibility");
    assertTrue(approxEqual(vis_crouching, 0.5f), "Crouching visibility = light * 0.5");
}

static void testStealthSprint() {
    std::cout << "\n=== FPSStealth: Sprint Noise ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->detection_decay_rate = 0.0f;

    sys.update(0.1f);
    float noise_walk = sys.getNoiseLevel("player_1");

    sys.setSprinting("player_1", true);
    sys.update(0.1f);
    float noise_sprint = sys.getNoiseLevel("player_1");

    assertTrue(noise_sprint > noise_walk, "Sprinting increases noise");
}

static void testStealthShadow() {
    std::cout << "\n=== FPSStealth: Shadow Zone ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->light_level = 1.0f;
    stealth->detection_decay_rate = 0.0f;

    sys.update(0.1f);
    float vis_lit = sys.getVisibility("player_1");

    sys.setInShadow("player_1", true);
    sys.update(0.1f);
    float vis_shadow = sys.getVisibility("player_1");

    assertTrue(vis_shadow < vis_lit, "Shadow reduces visibility");
}

static void testStealthTimesDetected() {
    std::cout << "\n=== FPSStealth: Times Detected/Escaped ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->detection_decay_rate = 0.0f;

    sys.addDetection("player_1", 0.8f);
    sys.update(0.1f);
    assertTrue(sys.getTimesDetected("player_1") == 1, "Detected once");

    sys.resetDetection("player_1");
    sys.update(0.1f);
    assertTrue(sys.getTimesEscaped("player_1") == 1, "Escaped once");
}

static void testStealthTimeHidden() {
    std::cout << "\n=== FPSStealth: Time Hidden ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* stealth = addComp<components::FPSStealth>(e);
    stealth->detection_decay_rate = 0.0f;

    sys.update(5.0f);
    assertTrue(approxEqual(sys.getTimeHidden("player_1"), 5.0f), "5s hidden");
}

static void testStealthLightLevel() {
    std::cout << "\n=== FPSStealth: Light Level ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::FPSStealth>(e);

    assertTrue(sys.setLightLevel("player_1", 0.2f), "Set light level");
    sys.update(0.1f);
    assertTrue(approxEqual(sys.getVisibility("player_1"), 0.2f), "Visibility matches light");
    assertTrue(!sys.setLightLevel("player_1", -0.1f), "Negative rejected");
    assertTrue(!sys.setLightLevel("player_1", 1.5f), "Above 1.0 rejected");
}

static void testStealthMissingEntity() {
    std::cout << "\n=== FPSStealth: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    assertTrue(sys.getDetectionState("nope") == 0, "State is 0");
    assertTrue(approxEqual(sys.getDetectionMeter("nope"), 0.0f), "Meter is 0");
    assertTrue(!sys.addDetection("nope", 0.5f), "addDetection fails");
    assertTrue(!sys.setCrouching("nope", true), "setCrouching fails");
    assertTrue(!sys.setSprinting("nope", true), "setSprinting fails");
    assertTrue(!sys.setInShadow("nope", true), "setInShadow fails");
    assertTrue(approxEqual(sys.getVisibility("nope"), 0.0f), "Visibility is 0");
    assertTrue(approxEqual(sys.getNoiseLevel("nope"), 0.0f), "Noise is 0");
}

static void testStealthNegativeDetection() {
    std::cout << "\n=== FPSStealth: Negative Detection ===" << std::endl;
    ecs::World world;
    systems::FPSStealthSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::FPSStealth>(e);

    assertTrue(!sys.addDetection("player_1", -0.5f), "Negative amount rejected");
    assertTrue(!sys.addDetection("player_1", 0.0f), "Zero amount rejected");
}

void run_fps_stealth_system_tests() {
    testStealthInitialState();
    testStealthDetectionMeter();
    testStealthDecay();
    testStealthCrouch();
    testStealthSprint();
    testStealthShadow();
    testStealthTimesDetected();
    testStealthTimeHidden();
    testStealthLightLevel();
    testStealthMissingEntity();
    testStealthNegativeDetection();
}
