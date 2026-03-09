// Tests for: Velocity Arc HUD System
#include "test_log.h"
#include "components/ui_components.h"
#include "systems/velocity_arc_hud_system.h"

using namespace atlas;

// ==================== Velocity Arc HUD System Tests ====================

static void testVelocityArcHudCreate() {
    std::cout << "\n=== VelocityArcHud: Create ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getSpeedPercent("ship1"), 0.0f), "Speed starts at 0");
    assertTrue(sys.getSpeedState("ship1") == 0, "Idle at 0 speed");
    assertTrue(!sys.isAfterburnerActive("ship1"), "No afterburner initially");
    assertTrue(approxEqual(sys.getWarpPrepProgress("ship1"), 0.0f), "No warp prep initially");
}

static void testVelocityArcHudSetSpeed() {
    std::cout << "\n=== VelocityArcHud: SetSpeed ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setSpeed("ship1", 50.0f, 100.0f), "Set speed succeeds");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getSpeedPercent("ship1"), 50.0f), "50% speed");
    assertTrue(sys.getSpeedState("ship1") == 1, "Normal state");
}

static void testVelocityArcHudApproaching() {
    std::cout << "\n=== VelocityArcHud: Approaching ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setSpeed("ship1", 85.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getSpeedState("ship1") == 2, "Approaching at 85%");
}

static void testVelocityArcHudAtMax() {
    std::cout << "\n=== VelocityArcHud: AtMax ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setSpeed("ship1", 100.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getSpeedState("ship1") == 3, "AtMax at 100%");
    assertTrue(approxEqual(sys.getSpeedPercent("ship1"), 100.0f), "100% speed");
}

static void testVelocityArcHudIdle() {
    std::cout << "\n=== VelocityArcHud: Idle ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setSpeed("ship1", 0.5f, 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getSpeedState("ship1") == 0, "Idle below threshold");
}

static void testVelocityArcHudAfterburner() {
    std::cout << "\n=== VelocityArcHud: Afterburner ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setAfterburner("ship1", true), "Enable afterburner");
    assertTrue(sys.isAfterburnerActive("ship1"), "Afterburner is active");
    assertTrue(sys.setAfterburner("ship1", false), "Disable afterburner");
    assertTrue(!sys.isAfterburnerActive("ship1"), "Afterburner is off");
}

static void testVelocityArcHudWarpPrep() {
    std::cout << "\n=== VelocityArcHud: WarpPrep ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setWarpPrepProgress("ship1", 0.6f), "Set warp prep");
    assertTrue(approxEqual(sys.getWarpPrepProgress("ship1"), 0.6f), "Warp prep is 0.6");
    sys.setWarpPrepProgress("ship1", 1.5f);
    assertTrue(approxEqual(sys.getWarpPrepProgress("ship1"), 1.0f), "Clamped to 1.0");
    sys.setWarpPrepProgress("ship1", -0.5f);
    assertTrue(approxEqual(sys.getWarpPrepProgress("ship1"), 0.0f), "Clamped to 0.0");
}

static void testVelocityArcHudSpeedClamp() {
    std::cout << "\n=== VelocityArcHud: SpeedClamp ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setSpeed("ship1", 200.0f, 100.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getSpeedPercent("ship1"), 100.0f), "Speed clamped to max");
}

static void testVelocityArcHudVisibility() {
    std::cout << "\n=== VelocityArcHud: Visibility ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.setVisible("ship1", false), "Set invisible succeeds");
    auto* entity = world.getEntity("ship1");
    auto* arc = entity->getComponent<components::VelocityArcHud>();
    assertTrue(!arc->visible, "Arc is invisible");
}

static void testVelocityArcHudMissing() {
    std::cout << "\n=== VelocityArcHud: Missing ===" << std::endl;
    ecs::World world;
    systems::VelocityArcHudSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.setSpeed("nonexistent", 50.0f, 100.0f), "Set speed fails on missing");
    assertTrue(!sys.setAfterburner("nonexistent", true), "Afterburner fails on missing");
    assertTrue(!sys.setWarpPrepProgress("nonexistent", 0.5f), "Warp prep fails on missing");
    assertTrue(approxEqual(sys.getSpeedPercent("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getSpeedState("nonexistent") == 0, "0 state on missing");
    assertTrue(!sys.isAfterburnerActive("nonexistent"), "No afterburner on missing");
    assertTrue(approxEqual(sys.getWarpPrepProgress("nonexistent"), 0.0f), "0 warp on missing");
    assertTrue(!sys.setVisible("nonexistent", true), "Set visible fails on missing");
}

void run_velocity_arc_hud_system_tests() {
    testVelocityArcHudCreate();
    testVelocityArcHudSetSpeed();
    testVelocityArcHudApproaching();
    testVelocityArcHudAtMax();
    testVelocityArcHudIdle();
    testVelocityArcHudAfterburner();
    testVelocityArcHudWarpPrep();
    testVelocityArcHudSpeedClamp();
    testVelocityArcHudVisibility();
    testVelocityArcHudMissing();
}
