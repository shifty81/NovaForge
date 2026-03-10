// Tests for: Blueprint Research System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/blueprint_research_system.h"

using namespace atlas;

// ==================== Blueprint Research System Tests ====================

static void testBlueprintResearchCreate() {
    std::cout << "\n=== BlueprintResearch: Create ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    assertTrue(sys.initialize("lab1", "station_lab_01"), "Init succeeds");
    assertTrue(sys.getJobCount("lab1") == 0, "No jobs");
    assertTrue(sys.getActiveJobCount("lab1") == 0, "No active jobs");
    assertTrue(sys.getTotalCompleted("lab1") == 0, "0 completed");
    assertTrue(sys.getTotalCancelled("lab1") == 0, "0 cancelled");
    assertTrue(approxEqual(sys.getResearchSpeed("lab1"), 1.0f), "Default speed 1.0");
}

static void testBlueprintResearchStartME() {
    std::cout << "\n=== BlueprintResearch: StartME ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    assertTrue(sys.startResearch("lab1", "bp_rifter",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 5, 300.0f), "Start ME research 0→5");
    assertTrue(sys.getJobCount("lab1") == 1, "1 job");
    assertTrue(sys.getActiveJobCount("lab1") == 1, "1 active");
    assertTrue(approxEqual(sys.getJobProgress("lab1", "bp_rifter"), 0.0f), "0 progress");
    assertTrue(!sys.isJobCompleted("lab1", "bp_rifter"), "Not completed");
}

static void testBlueprintResearchStartTE() {
    std::cout << "\n=== BlueprintResearch: StartTE ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    assertTrue(sys.startResearch("lab1", "bp_thorax",
        components::BlueprintResearchState::ResearchType::TimeEfficiency,
        0, 10, 600.0f), "Start TE research 0→10");
    assertTrue(sys.getJobCount("lab1") == 1, "1 job");
}

static void testBlueprintResearchComplete() {
    std::cout << "\n=== BlueprintResearch: Complete ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");
    sys.startResearch("lab1", "bp_rifter",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 5, 100.0f);

    // Progress through 50% of the time
    sys.update(50.0f);
    assertTrue(approxEqual(sys.getJobProgress("lab1", "bp_rifter"), 50.0f), "50% progress");
    assertTrue(!sys.isJobCompleted("lab1", "bp_rifter"), "Not complete at 50%");

    // Complete the rest
    sys.update(60.0f);
    assertTrue(sys.getTotalCompleted("lab1") == 1, "1 completed");
    // Completed jobs are removed after update
    assertTrue(sys.getJobCount("lab1") == 0, "Job removed after completion");
}

static void testBlueprintResearchSpeed() {
    std::cout << "\n=== BlueprintResearch: Speed ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");
    sys.setResearchSpeed("lab1", 2.0f); // 2x speed

    sys.startResearch("lab1", "bp_rifter",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 1, 100.0f);

    // At 2x speed, 50 seconds = 100 effective → complete
    sys.update(50.0f);
    assertTrue(sys.getTotalCompleted("lab1") == 1, "Completed in half time");
}

static void testBlueprintResearchCancel() {
    std::cout << "\n=== BlueprintResearch: Cancel ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");
    sys.startResearch("lab1", "bp_rifter",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 5, 300.0f);

    assertTrue(sys.cancelResearch("lab1", "bp_rifter"), "Cancel succeeds");
    assertTrue(sys.getTotalCancelled("lab1") == 1, "1 cancelled");

    sys.update(1.0f); // Remove cancelled jobs
    assertTrue(sys.getJobCount("lab1") == 0, "Cancelled job removed");
    assertTrue(!sys.cancelResearch("lab1", "bp_rifter"), "Double cancel fails");
}

static void testBlueprintResearchMaxJobs() {
    std::cout << "\n=== BlueprintResearch: MaxJobs ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    // Default max_concurrent_jobs = 3
    assertTrue(sys.startResearch("lab1", "bp_1",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 1, 100.0f), "Job 1");
    assertTrue(sys.startResearch("lab1", "bp_2",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 1, 100.0f), "Job 2");
    assertTrue(sys.startResearch("lab1", "bp_3",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 1, 100.0f), "Job 3");
    assertTrue(!sys.startResearch("lab1", "bp_4",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 1, 100.0f), "Job 4 rejected (max)");
}

static void testBlueprintResearchLevelBounds() {
    std::cout << "\n=== BlueprintResearch: LevelBounds ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    // ME max is 10
    assertTrue(!sys.startResearch("lab1", "bp_over",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 11, 100.0f), "ME > 10 rejected");

    // TE max is 20
    assertTrue(sys.startResearch("lab1", "bp_te_max",
        components::BlueprintResearchState::ResearchType::TimeEfficiency,
        0, 20, 100.0f), "TE = 20 OK");

    assertTrue(!sys.startResearch("lab1", "bp_te_over",
        components::BlueprintResearchState::ResearchType::TimeEfficiency,
        0, 21, 100.0f), "TE > 20 rejected");

    // target <= current rejected
    assertTrue(!sys.startResearch("lab1", "bp_back",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        5, 5, 100.0f), "target == current rejected");
    assertTrue(!sys.startResearch("lab1", "bp_back2",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        5, 3, 100.0f), "target < current rejected");
}

static void testBlueprintResearchDuplicate() {
    std::cout << "\n=== BlueprintResearch: Duplicate ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    assertTrue(sys.startResearch("lab1", "bp_rifter",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 5, 300.0f), "First research starts");
    assertTrue(!sys.startResearch("lab1", "bp_rifter",
        components::BlueprintResearchState::ResearchType::TimeEfficiency,
        0, 10, 300.0f), "Duplicate blueprint rejected");
}

static void testBlueprintResearchMissing() {
    std::cout << "\n=== BlueprintResearch: Missing ===" << std::endl;
    ecs::World world;
    systems::BlueprintResearchSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.startResearch("nonexistent", "bp",
        components::BlueprintResearchState::ResearchType::MaterialEfficiency,
        0, 1, 100.0f), "Start fails on missing");
    assertTrue(!sys.cancelResearch("nonexistent", "bp"), "Cancel fails on missing");
    assertTrue(!sys.setResearchSpeed("nonexistent", 1.0f), "SetSpeed fails on missing");
    assertTrue(sys.getJobCount("nonexistent") == 0, "0 jobs on missing");
    assertTrue(sys.getActiveJobCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(sys.getTotalCancelled("nonexistent") == 0, "0 cancelled on missing");
    assertTrue(approxEqual(sys.getJobProgress("nonexistent", "bp"), 0.0f), "0 progress on missing");
    assertTrue(!sys.isJobCompleted("nonexistent", "bp"), "Not completed on missing");
    assertTrue(approxEqual(sys.getResearchSpeed("nonexistent"), 0.0f), "0 speed on missing");
}

void run_blueprint_research_system_tests() {
    testBlueprintResearchCreate();
    testBlueprintResearchStartME();
    testBlueprintResearchStartTE();
    testBlueprintResearchComplete();
    testBlueprintResearchSpeed();
    testBlueprintResearchCancel();
    testBlueprintResearchMaxJobs();
    testBlueprintResearchLevelBounds();
    testBlueprintResearchDuplicate();
    testBlueprintResearchMissing();
}
