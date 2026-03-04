// Tests for: EntityStressTest Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/entity_stress_test_system.h"

using namespace atlas;

// ==================== EntityStressTest Tests ====================

static void testStressTestInit() {
    std::cout << "\n=== EntityStressTest: Init ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    assertTrue(sys.initializeStressTest("stress_1", "server_1", 500), "Stress test initialized");
    assertTrue(sys.getPhase("stress_1") == "Idle", "Phase is Idle");
    assertTrue(approxEqual(sys.getAverageTickMs("stress_1"), 0.0f), "Avg tick is 0");
    assertTrue(!sys.initializeStressTest("stress_1", "server_1", 500), "Duplicate init fails");
}

static void testStressTestStart() {
    std::cout << "\n=== EntityStressTest: Start ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    assertTrue(sys.startTest("stress_1"), "Test started");
    assertTrue(sys.getPhase("stress_1") == "Creating", "Phase is Creating");
    assertTrue(!sys.startTest("stress_1"), "Double start fails");
}

static void testStressTestRecordTick() {
    std::cout << "\n=== EntityStressTest: Record Tick ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    assertTrue(sys.recordTick("stress_1", 10.0f), "Tick recorded");
    assertTrue(sys.recordTick("stress_1", 20.0f), "Second tick recorded");
    assertTrue(sys.getAverageTickMs("stress_1") > 0.0f, "Avg tick > 0");
}

static void testStressTestRecordQuery() {
    std::cout << "\n=== EntityStressTest: Record Query ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    assertTrue(sys.recordQuery("stress_1", 50.0f), "Query recorded");
    assertTrue(sys.getAverageQueryUs("stress_1") > 0.0f, "Avg query > 0");
}

static void testStressTestEntityCount() {
    std::cout << "\n=== EntityStressTest: Entity Count ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    assertTrue(sys.setEntityCount("stress_1", 250), "Entity count set to 250");
    assertTrue(sys.getPhase("stress_1") == "Creating", "Still creating at 250");
    assertTrue(sys.setEntityCount("stress_1", 500), "Entity count set to 500");
    assertTrue(sys.getPhase("stress_1") == "Running", "Running at 500");
}

static void testStressTestBudget() {
    std::cout << "\n=== EntityStressTest: Budget ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 10.0f);
    sys.recordTick("stress_1", 15.0f);
    sys.recordTick("stress_1", 12.0f);
    sys.completeTest("stress_1");
    assertTrue(sys.isWithinBudget("stress_1"), "Within budget at ~12ms avg");
}

static void testStressTestOverBudget() {
    std::cout << "\n=== EntityStressTest: Over Budget ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 60.0f);
    sys.recordTick("stress_1", 70.0f);
    sys.completeTest("stress_1");
    assertTrue(!sys.isWithinBudget("stress_1"), "Over budget at 60-70ms avg");
}

static void testStressTestMaxTick() {
    std::cout << "\n=== EntityStressTest: Max Tick ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 5.0f);
    sys.recordTick("stress_1", 45.0f);
    sys.recordTick("stress_1", 10.0f);
    assertTrue(approxEqual(sys.getMaxTickMs("stress_1"), 45.0f), "Max tick is 45ms");
}

static void testStressTestComplete() {
    std::cout << "\n=== EntityStressTest: Complete ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 20.0f);
    assertTrue(sys.completeTest("stress_1"), "Test completed");
    assertTrue(sys.getPhase("stress_1") == "Complete", "Phase is Complete");
}

static void testStressTestMissing() {
    std::cout << "\n=== EntityStressTest: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    assertTrue(!sys.initializeStressTest("nonexistent", "s", 500), "Init fails on missing");
    assertTrue(approxEqual(sys.getAverageTickMs("nonexistent"), 0.0f), "Avg 0 on missing");
    assertTrue(approxEqual(sys.getMaxTickMs("nonexistent"), 0.0f), "Max 0 on missing");
    assertTrue(sys.getPhase("nonexistent").empty(), "Empty phase on missing");
    assertTrue(!sys.isWithinBudget("nonexistent"), "Not within budget on missing");
}


void run_entity_stress_test_tests() {
    testStressTestInit();
    testStressTestStart();
    testStressTestRecordTick();
    testStressTestRecordQuery();
    testStressTestEntityCount();
    testStressTestBudget();
    testStressTestOverBudget();
    testStressTestMaxTick();
    testStressTestComplete();
    testStressTestMissing();
}
