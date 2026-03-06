// Tests for: UndockSequenceSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/undock_sequence_system.h"

using namespace atlas;

// ==================== UndockSequenceSystem Tests ====================

static void testUndockRequest() {
    std::cout << "\n=== UndockSequence: Request ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::UndockSequence>(e);

    assertTrue(sys.getPhase("ship_1") == 0, "Initially Docked (phase 0)");
    assertTrue(sys.requestUndock("ship_1", "station_alpha"), "Undock request succeeds");
    assertTrue(sys.getPhase("ship_1") == 1, "Phase is RequestingUndock (1)");
    assertTrue(sys.isUndocking("ship_1"), "isUndocking is true");
    assertTrue(sys.getUndockCount("ship_1") == 1, "Undock count is 1");
}

static void testUndockPhaseProgression() {
    std::cout << "\n=== UndockSequence: Phase Progression ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);
    seq->phase_speed = 1.0f; // 1 second per phase

    sys.requestUndock("ship_1", "station_alpha");
    assertTrue(sys.getPhase("ship_1") == 1, "Phase 1: RequestingUndock");

    sys.update(1.1f);  // Complete phase 1 → phase 2
    assertTrue(sys.getPhase("ship_1") == 2, "Phase 2: HangarExit");

    sys.update(1.1f);  // Complete phase 2 → phase 3
    assertTrue(sys.getPhase("ship_1") == 3, "Phase 3: TunnelTraversal");

    sys.update(1.1f);  // Complete phase 3 → phase 4
    assertTrue(sys.getPhase("ship_1") == 4, "Phase 4: ExitAnimation");

    sys.update(1.1f);  // Complete phase 4 → phase 5
    assertTrue(sys.getPhase("ship_1") == 5, "Phase 5: Ejected");

    sys.update(1.1f);  // Complete phase 5 → phase 6
    assertTrue(sys.getPhase("ship_1") == 6, "Phase 6: Complete");
    assertTrue(!sys.isUndocking("ship_1"), "No longer undocking");
}

static void testUndockCancel() {
    std::cout << "\n=== UndockSequence: Cancel ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::UndockSequence>(e);

    sys.requestUndock("ship_1", "station_alpha");
    assertTrue(sys.cancelUndock("ship_1"), "Cancel succeeds in RequestingUndock");
    assertTrue(sys.getPhase("ship_1") == 0, "Back to Docked");
    assertTrue(!sys.isUndocking("ship_1"), "Not undocking");
}

static void testUndockCancelTooLate() {
    std::cout << "\n=== UndockSequence: Cancel Too Late ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);
    seq->phase_speed = 1.0f;

    sys.requestUndock("ship_1", "station_alpha");
    sys.update(1.1f);  // → HangarExit
    sys.update(1.1f);  // → TunnelTraversal

    assertTrue(sys.getPhase("ship_1") == 3, "Phase is TunnelTraversal");
    assertTrue(!sys.cancelUndock("ship_1"), "Cancel fails in TunnelTraversal");
    assertTrue(sys.getPhase("ship_1") == 3, "Phase unchanged");
}

static void testUndockInvulnerability() {
    std::cout << "\n=== UndockSequence: Invulnerability ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);
    seq->phase_speed = 10.0f;              // Fast progression
    seq->invulnerability_duration = 5.0f;  // 5s invuln

    sys.requestUndock("ship_1", "station_alpha");
    // Advance through all phases quickly
    sys.update(1.0f); sys.update(1.0f); sys.update(1.0f);
    sys.update(1.0f); sys.update(1.0f);

    assertTrue(sys.getPhase("ship_1") == 6, "Reached Complete");
    assertTrue(sys.isInvulnerable("ship_1"), "Is invulnerable after undock");

    sys.update(3.0f);
    assertTrue(sys.isInvulnerable("ship_1"), "Still invulnerable at 3s");

    sys.update(3.0f);
    assertTrue(!sys.isInvulnerable("ship_1"), "Invulnerability expired at 6s");
}

static void testUndockSetExitPosition() {
    std::cout << "\n=== UndockSequence: Set Exit Position ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);

    assertTrue(sys.setExitPosition("ship_1", 100.0f, 200.0f, 300.0f), "Set position succeeds");
    assertTrue(approxEqual(seq->exit_x, 100.0f), "Exit X is 100");
    assertTrue(approxEqual(seq->exit_y, 200.0f), "Exit Y is 200");
    assertTrue(approxEqual(seq->exit_z, 300.0f), "Exit Z is 300");
}

static void testUndockSetPhaseSpeed() {
    std::cout << "\n=== UndockSequence: Set Phase Speed ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);

    assertTrue(sys.setPhaseSpeed("ship_1", 2.0f), "Set speed succeeds");
    assertTrue(approxEqual(seq->phase_speed, 2.0f), "Phase speed is 2.0");
    assertTrue(!sys.setPhaseSpeed("ship_1", -1.0f), "Negative speed rejected");
    assertTrue(!sys.setPhaseSpeed("ship_1", 0.0f), "Zero speed rejected");
}

static void testUndockAlreadyUndocking() {
    std::cout << "\n=== UndockSequence: Already Undocking ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::UndockSequence>(e);

    assertTrue(sys.requestUndock("ship_1", "station_a"), "First undock succeeds");
    assertTrue(!sys.requestUndock("ship_1", "station_b"), "Second undock fails");
    assertTrue(sys.getPhase("ship_1") == 1, "Phase still RequestingUndock");
}

static void testUndockMissingEntity() {
    std::cout << "\n=== UndockSequence: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    assertTrue(!sys.requestUndock("nonexistent", "station"), "Request fails");
    assertTrue(!sys.cancelUndock("nonexistent"), "Cancel fails");
    assertTrue(sys.getPhase("nonexistent") == 0, "Phase is 0");
    assertTrue(approxEqual(sys.getProgress("nonexistent"), 0.0f), "Progress is 0");
    assertTrue(!sys.isUndocking("nonexistent"), "Not undocking");
    assertTrue(!sys.isInvulnerable("nonexistent"), "Not invulnerable");
    assertTrue(approxEqual(sys.getTotalUndockTime("nonexistent"), 0.0f), "Time is 0");
    assertTrue(sys.getUndockCount("nonexistent") == 0, "Count is 0");
}

static void testUndockTotalTime() {
    std::cout << "\n=== UndockSequence: Total Time ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);
    seq->phase_speed = 1.0f;

    sys.requestUndock("ship_1", "station_alpha");
    sys.update(0.5f);
    assertTrue(approxEqual(sys.getTotalUndockTime("ship_1"), 0.5f), "Total time is 0.5s");

    sys.update(0.5f);
    assertTrue(approxEqual(sys.getTotalUndockTime("ship_1"), 1.0f), "Total time is 1.0s");
}

static void testUndockMultipleUndocks() {
    std::cout << "\n=== UndockSequence: Multiple Undocks ===" << std::endl;
    ecs::World world;
    systems::UndockSequenceSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* seq = addComp<components::UndockSequence>(e);
    seq->phase_speed = 10.0f;

    sys.requestUndock("ship_1", "station_a");
    // Complete undock
    for (int i = 0; i < 6; i++) sys.update(1.0f);
    assertTrue(sys.getPhase("ship_1") == 6, "First undock complete");
    assertTrue(sys.getUndockCount("ship_1") == 1, "Count is 1");

    // Reset to docked for second undock
    seq->phase = components::UndockSequence::Docked;
    seq->is_invulnerable = false;
    assertTrue(sys.requestUndock("ship_1", "station_b"), "Second undock succeeds");
    assertTrue(sys.getUndockCount("ship_1") == 2, "Count is 2");
}

void run_undock_sequence_system_tests() {
    testUndockRequest();
    testUndockPhaseProgression();
    testUndockCancel();
    testUndockCancelTooLate();
    testUndockInvulnerability();
    testUndockSetExitPosition();
    testUndockSetPhaseSpeed();
    testUndockAlreadyUndocking();
    testUndockMissingEntity();
    testUndockTotalTime();
    testUndockMultipleUndocks();
}
