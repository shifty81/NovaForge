// Tests for: JumpDriveSystem Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/jump_drive_system.h"

using namespace atlas;

// ==================== JumpDriveSystem Tests ====================

static void testJumpDriveInitiate() {
    std::cout << "\n=== Jump Drive: Initiate Jump ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->max_range_ly = 5.0f;
    jd->fuel_per_ly = 500.0f;
    jd->current_fuel = 5000.0f;
    jd->requires_cyno = true;

    systems::JumpDriveSystem sys(&world);
    assertTrue(sys.getPhase("capital1") == "idle", "Initial phase is idle");
    assertTrue(sys.initiateJump("capital1", "SystemB", 3.0f, "cyno1"), "Jump initiated with cyno");
    assertTrue(sys.getPhase("capital1") == "spooling_up", "Phase is spooling_up");
    assertTrue(!sys.initiateJump("capital1", "SystemC", 2.0f, "cyno2"), "Cannot initiate while spooling");
}

static void testJumpDriveNoCyno() {
    std::cout << "\n=== Jump Drive: No Cyno Required ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->requires_cyno = true;
    jd->current_fuel = 5000.0f;

    systems::JumpDriveSystem sys(&world);
    assertTrue(!sys.initiateJump("capital1", "SystemB", 3.0f, ""), "Jump fails without cyno when required");
    jd->requires_cyno = false;
    assertTrue(sys.initiateJump("capital1", "SystemB", 3.0f, ""), "Jump succeeds without cyno when not required");
}

static void testJumpDriveFullCycle() {
    std::cout << "\n=== Jump Drive: Full Cycle ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->spool_time = 5.0f;
    jd->cooldown_time = 10.0f;
    jd->fuel_per_ly = 500.0f;
    jd->current_fuel = 5000.0f;
    jd->max_range_ly = 5.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    sys.initiateJump("capital1", "SystemB", 3.0f);
    sys.update(5.0f);  // spool complete → Jumping phase, fuel consumed
    assertTrue(sys.getPhase("capital1") == "jumping", "Phase is jumping after spool");
    assertTrue(sys.getTotalJumps("capital1") == 1, "1 jump completed");
    assertTrue(approxEqual(sys.getFuel("capital1"), 3500.0f), "Fuel consumed: 5000 - 500*3 = 3500");

    sys.update(0.1f);  // Jumping → Cooldown
    assertTrue(sys.getPhase("capital1") == "cooldown", "Phase is cooldown");

    sys.update(10.0f);  // Cooldown complete
    assertTrue(sys.getPhase("capital1") == "idle", "Phase returns to idle");
}

static void testJumpDriveFuelCheck() {
    std::cout << "\n=== Jump Drive: Fuel Check ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->fuel_per_ly = 500.0f;
    jd->current_fuel = 1000.0f;
    jd->max_range_ly = 5.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    assertTrue(!sys.initiateJump("capital1", "SystemFar", 3.0f), "Jump fails: not enough fuel for 3 LY");
    assertTrue(sys.initiateJump("capital1", "SystemNear", 2.0f), "Jump succeeds: enough fuel for 2 LY");
}

static void testJumpDriveFatigue() {
    std::cout << "\n=== Jump Drive: Fatigue ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->spool_time = 1.0f;
    jd->cooldown_time = 1.0f;
    jd->fuel_per_ly = 100.0f;
    jd->current_fuel = 10000.0f;
    jd->max_range_ly = 5.0f;
    jd->max_fatigue = 10.0f;
    jd->fatigue_per_jump = 3.0f;
    jd->fatigue_decay_rate = 0.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    // Jump 1: fatigue 0 → 3
    sys.initiateJump("capital1", "S1", 1.0f);
    sys.update(1.0f);  // spool → jump
    sys.update(0.1f);  // jump → cooldown
    sys.update(1.0f);  // cooldown → idle
    assertTrue(approxEqual(sys.getFatigue("capital1"), 3.0f), "Fatigue after 1 jump");

    // Jump 2: fatigue 3 → 6
    sys.initiateJump("capital1", "S2", 1.0f);
    sys.update(1.0f);
    sys.update(0.1f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getFatigue("capital1"), 6.0f), "Fatigue after 2 jumps");

    // Jump 3: fatigue 6 → 9
    sys.initiateJump("capital1", "S3", 1.0f);
    sys.update(1.0f);
    sys.update(0.1f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getFatigue("capital1"), 9.0f), "Fatigue after 3 jumps");

    // Jump 4: fatigue 9 → would be 12 but capped at 10, also effective range reduced
    // effective range = 5 * (1 - 9/10) = 0.5 LY — can still jump 0.4 LY
    assertTrue(sys.canJump("capital1", 0.4f), "Can jump short distance with high fatigue");
}

static void testJumpDriveFatigueDecay() {
    std::cout << "\n=== Jump Drive: Fatigue Decay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->fatigue_hours = 5.0f;
    jd->fatigue_decay_rate = 1.0f;
    jd->max_fatigue = 10.0f;

    systems::JumpDriveSystem sys(&world);
    sys.update(3.0f);  // decay: 5 - 1*3 = 2
    assertTrue(approxEqual(sys.getFatigue("capital1"), 2.0f), "Fatigue decayed to 2");
    sys.update(5.0f);  // decay: 2 - 5 → clamped to 0
    assertTrue(approxEqual(sys.getFatigue("capital1"), 0.0f), "Fatigue decayed to 0 (clamped)");
}

static void testJumpDriveCancel() {
    std::cout << "\n=== Jump Drive: Cancel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->current_fuel = 5000.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    sys.initiateJump("capital1", "SystemB", 2.0f);
    assertTrue(sys.cancelJump("capital1"), "Cancel during spool succeeds");
    assertTrue(sys.getPhase("capital1") == "idle", "Phase back to idle");
    assertTrue(!sys.cancelJump("capital1"), "Cancel in idle fails");
}

static void testJumpDriveRefuel() {
    std::cout << "\n=== Jump Drive: Refuel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->current_fuel = 2000.0f;
    jd->max_fuel = 10000.0f;

    systems::JumpDriveSystem sys(&world);
    assertTrue(sys.refuel("capital1", 3000.0f), "Refuel success");
    assertTrue(approxEqual(sys.getFuel("capital1"), 5000.0f), "Fuel is 5000 after refuel");
    sys.refuel("capital1", 20000.0f);
    assertTrue(approxEqual(sys.getFuel("capital1"), 10000.0f), "Fuel capped at max");
}

static void testJumpDriveEffectiveRange() {
    std::cout << "\n=== Jump Drive: Effective Range ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->max_range_ly = 10.0f;
    jd->max_fatigue = 10.0f;
    jd->fatigue_hours = 0.0f;

    systems::JumpDriveSystem sys(&world);
    assertTrue(approxEqual(sys.getEffectiveRange("capital1"), 10.0f), "Full range with no fatigue");
    jd->fatigue_hours = 5.0f;
    assertTrue(approxEqual(sys.getEffectiveRange("capital1"), 5.0f), "Half range at 50% fatigue");
    jd->fatigue_hours = 10.0f;
    assertTrue(approxEqual(sys.getEffectiveRange("capital1"), 0.0f), "Zero range at max fatigue");
}

static void testJumpDriveCooldownRemaining() {
    std::cout << "\n=== Jump Drive: Cooldown Remaining ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->spool_time = 1.0f;
    jd->cooldown_time = 10.0f;
    jd->current_fuel = 5000.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    assertTrue(approxEqual(sys.getCooldownRemaining("capital1"), 0.0f), "No cooldown in idle");
    sys.initiateJump("capital1", "S1", 1.0f);
    sys.update(1.0f);  // spool → jump
    sys.update(0.1f);  // jump → cooldown
    assertTrue(sys.getCooldownRemaining("capital1") > 9.0f, "Cooldown remaining after jump");
}

static void testJumpDriveMissing() {
    std::cout << "\n=== Jump Drive: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::JumpDriveSystem sys(&world);
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Default phase for missing");
    assertTrue(approxEqual(sys.getFuel("nonexistent"), 0.0f), "Default fuel for missing");
    assertTrue(approxEqual(sys.getMaxFuel("nonexistent"), 0.0f), "Default max fuel for missing");
    assertTrue(approxEqual(sys.getFatigue("nonexistent"), 0.0f), "Default fatigue for missing");
    assertTrue(approxEqual(sys.getMaxRange("nonexistent"), 0.0f), "Default max range for missing");
    assertTrue(approxEqual(sys.getEffectiveRange("nonexistent"), 0.0f), "Default effective range for missing");
    assertTrue(!sys.canJump("nonexistent", 1.0f), "Cannot jump for missing");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "Default total jumps for missing");
    assertTrue(approxEqual(sys.getCooldownRemaining("nonexistent"), 0.0f), "Default cooldown for missing");
}


void run_jump_drive_system_tests() {
    testJumpDriveInitiate();
    testJumpDriveNoCyno();
    testJumpDriveFullCycle();
    testJumpDriveFuelCheck();
    testJumpDriveFatigue();
    testJumpDriveFatigueDecay();
    testJumpDriveCancel();
    testJumpDriveRefuel();
    testJumpDriveEffectiveRange();
    testJumpDriveCooldownRemaining();
    testJumpDriveMissing();
}
