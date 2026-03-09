// Tests for: EntityStressTestSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/entity_stress_test_system.h"

using namespace atlas;

// ==================== EntityStressTestSystem Tests ====================

static void testStressTestInitialize() {
    std::cout << "\n=== EntityStressTest: Initialize ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");

    assertTrue(sys.initializeStressTest("stress1", "server_main", 500), "Initialize stress test");
    assertTrue(sys.getPhase("stress1") == "Idle", "Phase is Idle");
    assertTrue(sys.getAverageTickMs("stress1") == 0.0f, "Avg tick is 0");
    assertTrue(sys.getMaxTickMs("stress1") == 0.0f, "Max tick is 0");
    assertTrue(!sys.isWithinBudget("stress1"), "Not within budget initially");
}

static void testStressTestDuplicateInitRejected() {
    std::cout << "\n=== EntityStressTest: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");

    assertTrue(sys.initializeStressTest("stress1", "server_main", 500), "First init ok");
    assertTrue(!sys.initializeStressTest("stress1", "server_2", 100), "Duplicate init rejected");
}

static void testStressTestStartTest() {
    std::cout << "\n=== EntityStressTest: StartTest ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);

    assertTrue(sys.startTest("stress1"), "Start test");
    assertTrue(sys.getPhase("stress1") == "Creating", "Phase is Creating");
}

static void testStressTestCannotStartWhileRunning() {
    std::cout << "\n=== EntityStressTest: CannotStartWhileRunning ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");

    assertTrue(!sys.startTest("stress1"), "Cannot start while already running");
}

static void testStressTestRecordTick() {
    std::cout << "\n=== EntityStressTest: RecordTick ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");

    assertTrue(sys.recordTick("stress1", 10.0f), "Record first tick");
    assertTrue(sys.getAverageTickMs("stress1") == 10.0f, "Avg tick is 10ms");
    assertTrue(sys.getMaxTickMs("stress1") == 10.0f, "Max tick is 10ms");

    assertTrue(sys.recordTick("stress1", 20.0f), "Record second tick");
    assertTrue(approxEqual(sys.getAverageTickMs("stress1"), 15.0f), "Avg tick is 15ms");
    assertTrue(sys.getMaxTickMs("stress1") == 20.0f, "Max tick is 20ms");
}

static void testStressTestRecordQuery() {
    std::cout << "\n=== EntityStressTest: RecordQuery ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");

    assertTrue(sys.recordQuery("stress1", 50.0f), "Record query");
    assertTrue(sys.getAverageQueryUs("stress1") == 50.0f, "Avg query is 50us");

    assertTrue(sys.recordQuery("stress1", 100.0f), "Record second query");
    assertTrue(sys.getAverageQueryUs("stress1") > 50.0f, "Avg query increased");
}

static void testStressTestSetEntityCount() {
    std::cout << "\n=== EntityStressTest: SetEntityCount ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");

    assertTrue(sys.setEntityCount("stress1", 250), "Set entity count 250");
    assertTrue(sys.getPhase("stress1") == "Creating", "Still creating at 250");

    assertTrue(sys.setEntityCount("stress1", 500), "Set entity count 500");
    assertTrue(sys.getPhase("stress1") == "Running", "Transitioned to Running at target");
}

static void testStressTestPhaseTransitionViaRecordTick() {
    std::cout << "\n=== EntityStressTest: PhaseTransitionViaRecordTick ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 100);
    sys.startTest("stress1");
    sys.setEntityCount("stress1", 100);

    assertTrue(sys.getPhase("stress1") == "Running", "Phase is Running");
    sys.recordTick("stress1", 5.0f);
    assertTrue(sys.getPhase("stress1") == "Running", "Still running after tick");
}

static void testStressTestCompleteTest() {
    std::cout << "\n=== EntityStressTest: CompleteTest ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");
    sys.recordTick("stress1", 30.0f);
    sys.recordTick("stress1", 40.0f);

    assertTrue(sys.completeTest("stress1"), "Complete test");
    assertTrue(sys.getPhase("stress1") == "Complete", "Phase is Complete");
    assertTrue(approxEqual(sys.getAverageTickMs("stress1"), 35.0f), "Final avg is 35ms");
    assertTrue(sys.getMaxTickMs("stress1") == 40.0f, "Final max is 40ms");
}

static void testStressTestWithinBudget() {
    std::cout << "\n=== EntityStressTest: WithinBudget ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");

    // Record ticks well within default 50ms threshold
    sys.recordTick("stress1", 10.0f);
    sys.recordTick("stress1", 15.0f);
    sys.completeTest("stress1");

    assertTrue(sys.isWithinBudget("stress1"), "Within budget at ~12.5ms avg");
}

static void testStressTestOverBudget() {
    std::cout << "\n=== EntityStressTest: OverBudget ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");

    // Record ticks over 50ms threshold
    sys.recordTick("stress1", 60.0f);
    sys.recordTick("stress1", 70.0f);
    sys.completeTest("stress1");

    assertTrue(!sys.isWithinBudget("stress1"), "Over budget at ~65ms avg");
}

static void testStressTestUpdateRecomputesStats() {
    std::cout << "\n=== EntityStressTest: UpdateRecomputesStats ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress1");
    sys.initializeStressTest("stress1", "server_main", 500);
    sys.startTest("stress1");
    sys.recordTick("stress1", 20.0f);
    sys.recordTick("stress1", 40.0f);

    sys.update(0.016f);

    assertTrue(approxEqual(sys.getAverageTickMs("stress1"), 30.0f), "Update recomputed avg to 30ms");
    assertTrue(sys.getMaxTickMs("stress1") == 40.0f, "Update recomputed max to 40ms");
}

static void testStressTestMissingEntity() {
    std::cout << "\n=== EntityStressTest: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);

    assertTrue(!sys.initializeStressTest("ghost", "s1", 500), "Init fails for missing entity");
    assertTrue(!sys.startTest("ghost"), "startTest fails for missing");
    assertTrue(!sys.completeTest("ghost"), "completeTest fails for missing");
    assertTrue(!sys.recordTick("ghost", 10.0f), "recordTick fails for missing");
    assertTrue(!sys.recordQuery("ghost", 50.0f), "recordQuery fails for missing");
    assertTrue(!sys.setEntityCount("ghost", 100), "setEntityCount fails for missing");
    assertTrue(sys.getAverageTickMs("ghost") == 0.0f, "getAverageTickMs 0 for missing");
    assertTrue(sys.getMaxTickMs("ghost") == 0.0f, "getMaxTickMs 0 for missing");
    assertTrue(sys.getAverageQueryUs("ghost") == 0.0f, "getAverageQueryUs 0 for missing");
    assertTrue(!sys.isWithinBudget("ghost"), "isWithinBudget false for missing");
    assertTrue(sys.getPhase("ghost").empty(), "getPhase empty for missing");
}

void run_entity_stress_test_system_tests() {
    testStressTestInitialize();
    testStressTestDuplicateInitRejected();
    testStressTestStartTest();
    testStressTestCannotStartWhileRunning();
    testStressTestRecordTick();
    testStressTestRecordQuery();
    testStressTestSetEntityCount();
    testStressTestPhaseTransitionViaRecordTick();
    testStressTestCompleteTest();
    testStressTestWithinBudget();
    testStressTestOverBudget();
    testStressTestUpdateRecomputesStats();
    testStressTestMissingEntity();
}
