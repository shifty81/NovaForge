// Tests for: OreProcessing System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/ore_processing_system.h"

using namespace atlas;

// ==================== OreProcessing System Tests ====================

static void testOreProcessingCreate() {
    std::cout << "\n=== OreProcessing: Create ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeProcessing("ship1", 0.8f, 3), "Init processing succeeds");
    assertTrue(sys.getActiveJobCount("ship1") == 0, "No active jobs initially");
    assertTrue(approxEqual(sys.getTotalRefined("ship1"), 0.0f), "0 total refined");
    assertTrue(sys.getBatchesCompleted("ship1") == 0, "0 batches completed");
}

static void testOreProcessingQueue() {
    std::cout << "\n=== OreProcessing: Queue Ore ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProcessing("ship1", 0.75f, 2);

    assertTrue(sys.queueOre("ship1", "Veldspar", 1000.0f, 10.0f), "Queue first ore batch");
    assertTrue(sys.getActiveJobCount("ship1") == 1, "1 active job");
    assertTrue(sys.queueOre("ship1", "Scordite", 500.0f, 15.0f), "Queue second ore batch");
    assertTrue(sys.getActiveJobCount("ship1") == 2, "2 active jobs");
    assertTrue(!sys.queueOre("ship1", "Pyroxeres", 300.0f), "Third job rejected (max=2)");
}

static void testOreProcessingInvalidQueue() {
    std::cout << "\n=== OreProcessing: Invalid Queue ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProcessing("ship1");

    assertTrue(!sys.queueOre("ship1", "", 100.0f), "Empty ore type rejected");
    assertTrue(!sys.queueOre("ship1", "Veldspar", 0.0f), "Zero amount rejected");
    assertTrue(!sys.queueOre("ship1", "Veldspar", -10.0f), "Negative amount rejected");
    assertTrue(!sys.queueOre("nonexistent", "Veldspar", 100.0f), "Missing entity rejected");
}

static void testOreProcessingCompletion() {
    std::cout << "\n=== OreProcessing: Job Completion ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProcessing("ship1", 0.80f, 2);
    sys.queueOre("ship1", "Veldspar", 1000.0f, 10.0f);

    // Simulate 10 seconds (should complete the job)
    for (int i = 0; i < 100; ++i) sys.update(0.1f);

    assertTrue(sys.getActiveJobCount("ship1") == 0, "Job completed (0 active)");
    assertTrue(sys.getBatchesCompleted("ship1") == 1, "1 batch completed");
    assertTrue(approxEqual(sys.getTotalRefined("ship1"), 800.0f), "80% yield: 800 refined");
}

static void testOreProcessingEfficiency() {
    std::cout << "\n=== OreProcessing: Efficiency Change ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProcessing("ship1", 0.50f, 2);

    assertTrue(sys.setEfficiency("ship1", 0.90f), "Set efficiency to 90%");
    sys.queueOre("ship1", "Veldspar", 1000.0f, 5.0f);

    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(approxEqual(sys.getTotalRefined("ship1"), 900.0f), "90% yield: 900 refined");
}

static void testOreProcessingSpeed() {
    std::cout << "\n=== OreProcessing: Speed Multiplier ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProcessing("ship1", 1.0f, 2);
    sys.setProcessingSpeed("ship1", 2.0f);
    sys.queueOre("ship1", "Scordite", 500.0f, 10.0f);

    // At 2x speed, 10s job should complete in 5 seconds
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(sys.getBatchesCompleted("ship1") == 1, "2x speed: batch done in 5s");
    assertTrue(approxEqual(sys.getTotalRefined("ship1"), 500.0f), "100% eff: 500 refined");
}

static void testOreProcessingDuplicateInit() {
    std::cout << "\n=== OreProcessing: Duplicate Init ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeProcessing("ship1"), "First init succeeds");
    assertTrue(!sys.initializeProcessing("ship1"), "Duplicate init rejected");
}

static void testOreProcessingMultipleBatches() {
    std::cout << "\n=== OreProcessing: Multiple Batches ===" << std::endl;
    ecs::World world;
    systems::OreProcessingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProcessing("ship1", 0.75f, 3);
    sys.queueOre("ship1", "Veldspar", 100.0f, 2.0f);
    sys.queueOre("ship1", "Scordite", 200.0f, 2.0f);

    // Both should complete after 2 seconds
    for (int i = 0; i < 20; ++i) sys.update(0.1f);

    assertTrue(sys.getBatchesCompleted("ship1") == 2, "2 batches completed");
    assertTrue(approxEqual(sys.getTotalRefined("ship1"), 225.0f), "75% of 300 = 225");
}

void run_ore_processing_system_tests() {
    testOreProcessingCreate();
    testOreProcessingQueue();
    testOreProcessingInvalidQueue();
    testOreProcessingCompletion();
    testOreProcessingEfficiency();
    testOreProcessingSpeed();
    testOreProcessingDuplicateInit();
    testOreProcessingMultipleBatches();
}
