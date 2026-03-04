// Tests for: TerraformingSystem Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/terraforming_system.h"

using namespace atlas;

// ==================== TerraformingSystem Tests ====================

static void testTerraformingStart() {
    std::cout << "\n=== Terraforming: Start ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    assertTrue(sys.startTerraforming("planet1", "kepler-442b"), "Terraforming started");
    assertTrue(sys.isActive("planet1"), "Terraforming is active");
    assertTrue(sys.getStage("planet1") == "planning", "Stage is planning");
    assertTrue(!sys.startTerraforming("planet1", "kepler-442b"), "Duplicate start rejected");
}

static void testTerraformingPause() {
    std::cout << "\n=== Terraforming: Pause ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.pauseTerraforming("planet1"), "Terraforming paused");
    assertTrue(!sys.isActive("planet1"), "Terraforming is inactive");
}

static void testTerraformingResume() {
    std::cout << "\n=== Terraforming: Resume ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    sys.pauseTerraforming("planet1");
    assertTrue(sys.resumeTerraforming("planet1"), "Terraforming resumed");
    assertTrue(sys.isActive("planet1"), "Terraforming is active again");
}

static void testTerraformingCancel() {
    std::cout << "\n=== Terraforming: Cancel ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.cancelTerraforming("planet1"), "Terraforming cancelled");
    assertTrue(sys.getStage("planet1") == "unknown", "No stage after cancel");
    assertTrue(!sys.cancelTerraforming("planet1"), "Cancel on missing fails");
}

static void testTerraformingStageAdvance() {
    std::cout << "\n=== Terraforming: Stage Advance ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.getStage("planet1") == "planning", "Starts at planning");
    sys.advanceStage("planet1");
    assertTrue(sys.getStage("planet1") == "infrastructure", "Advanced to infrastructure");
    sys.advanceStage("planet1");
    assertTrue(sys.getStage("planet1") == "atmosphere_processing", "Advanced to atmosphere_processing");
}

static void testTerraformingAutoAdvance() {
    std::cout << "\n=== Terraforming: Auto Advance ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    auto* entity = world.getEntity("planet1");
    auto* tf = entity->getComponent<components::Terraforming>();
    tf->time_per_stage = 10.0f; // speed up for test

    sys.update(11.0f); // past one stage
    assertTrue(sys.getStage("planet1") == "infrastructure", "Auto-advanced to infrastructure");
}

static void testTerraformingSetTargets() {
    std::cout << "\n=== Terraforming: Set Targets ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.setTargets("planet1", 0.8f, 300.0f, 0.5f), "Targets set");
    assertTrue(!sys.setTargets("nonexistent", 0.8f, 300.0f, 0.5f), "Missing entity fails");
}

static void testTerraformingProgress() {
    std::cout << "\n=== Terraforming: Progress ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    auto* entity = world.getEntity("planet1");
    auto* tf = entity->getComponent<components::Terraforming>();
    tf->time_per_stage = 100.0f;

    sys.update(50.0f); // 50% through first stage
    assertTrue(approxEqual(sys.getProgress("planet1"), 0.5f), "Progress is 0.5 in stage");
    assertTrue(sys.getTotalProgress("planet1") > 0.0f, "Total progress > 0");
    assertTrue(sys.getTotalCreditsSpent("planet1") > 0.0, "Credits spent > 0");
}

static void testTerraformingComplete() {
    std::cout << "\n=== Terraforming: Complete ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    // Advance through all stages to completion
    sys.advanceStage("planet1"); // -> infrastructure
    sys.advanceStage("planet1"); // -> atmosphere_processing
    sys.advanceStage("planet1"); // -> temperature_regulation
    sys.advanceStage("planet1"); // -> biome_seeding
    sys.advanceStage("planet1"); // -> complete
    assertTrue(sys.getStage("planet1") == "complete", "Stage is complete");
    assertTrue(approxEqual(sys.getTotalProgress("planet1"), 1.0f), "Total progress is 1.0");
    assertTrue(!sys.advanceStage("planet1"), "Cannot advance past complete");
}

static void testTerraformingMissing() {
    std::cout << "\n=== Terraforming: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::TerraformingSystem sys(&world);
    assertTrue(sys.getStage("nonexistent") == "unknown", "Default stage for missing");
    assertTrue(approxEqual(sys.getProgress("nonexistent"), 0.0f), "Default progress for missing");
    assertTrue(approxEqual(sys.getTotalProgress("nonexistent"), 0.0f), "Default total progress for missing");
    assertTrue(!sys.isActive("nonexistent"), "Default inactive for missing");
    assertTrue(sys.getTotalCreditsSpent("nonexistent") == 0.0, "Default credits for missing");
}


void run_terraforming_system_tests() {
    testTerraformingStart();
    testTerraformingPause();
    testTerraformingResume();
    testTerraformingCancel();
    testTerraformingStageAdvance();
    testTerraformingAutoAdvance();
    testTerraformingSetTargets();
    testTerraformingProgress();
    testTerraformingComplete();
    testTerraformingMissing();
}
