// Tests for: Invention System (T2 Blueprint Invention)
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/invention_system.h"

using namespace atlas;

// ==================== Invention System Tests ====================

static void testInventionCreate() {
    std::cout << "\n=== Invention: Create ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    assertTrue(sys.initialize("lab1", "research_lab_01"), "Init succeeds");
    assertTrue(sys.getJobCount("lab1") == 0, "No jobs initially");
    assertTrue(sys.getActiveJobCount("lab1") == 0, "No active jobs");
    assertTrue(sys.getTotalAttempted("lab1") == 0, "0 attempted");
    assertTrue(sys.getTotalSucceeded("lab1") == 0, "0 succeeded");
    assertTrue(sys.getTotalFailed("lab1") == 0, "0 failed");
    assertTrue(sys.getTotalCancelled("lab1") == 0, "0 cancelled");
    assertTrue(approxEqual(sys.getResearchSpeed("lab1"), 1.0f), "Speed 1.0");
}

static void testInventionStartJob() {
    std::cout << "\n=== Invention: StartJob ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    assertTrue(sys.startJob("lab1", "job_1", "rifter_bp",
                            "datacore_mech", "datacore_elec", 0.25f, 100.0f),
               "Start job");
    assertTrue(sys.getJobCount("lab1") == 1, "1 job");
    assertTrue(sys.getActiveJobCount("lab1") == 1, "1 active");
    assertTrue(approxEqual(sys.getJobProgress("lab1", "job_1"), 0.0f), "0 progress");
    assertTrue(!sys.isJobCompleted("lab1", "job_1"), "Not completed");
}

static void testInventionProgress() {
    std::cout << "\n=== Invention: Progress ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");
    sys.startJob("lab1", "job_1", "rifter_bp",
                 "datacore_mech", "datacore_elec", 0.25f, 100.0f);

    // Tick 50 seconds at speed 1.0
    sys.update(50.0f);
    assertTrue(approxEqual(sys.getJobProgress("lab1", "job_1"), 50.0f), "50s progress");
    assertTrue(!sys.isJobCompleted("lab1", "job_1"), "Not completed at 50s");

    // Tick remaining
    sys.update(55.0f);
    assertTrue(sys.isJobCompleted("lab1", "job_1"), "Completed at 105s");
    assertTrue(sys.getTotalAttempted("lab1") == 1, "1 attempted");
}

static void testInventionSuccessHighChance() {
    std::cout << "\n=== Invention: SuccessHighChance ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    // base_chance=1.0 guarantees success
    sys.startJob("lab1", "job_1", "rifter_bp",
                 "datacore_mech", "datacore_elec", 1.0f, 50.0f);
    sys.update(55.0f);
    assertTrue(sys.isJobCompleted("lab1", "job_1"), "Completed");
    assertTrue(sys.getTotalSucceeded("lab1") == 1, "1 succeeded");
    assertTrue(sys.getTotalFailed("lab1") == 0, "0 failed");
}

static void testInventionFailLowChance() {
    std::cout << "\n=== Invention: FailLowChance ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    // base_chance=0.0 guarantees failure
    sys.startJob("lab1", "job_1", "rifter_bp",
                 "datacore_mech", "datacore_elec", 0.0f, 50.0f);
    sys.update(55.0f);
    assertTrue(sys.isJobCompleted("lab1", "job_1"), "Completed");
    assertTrue(sys.getTotalSucceeded("lab1") == 0, "0 succeeded");
    assertTrue(sys.getTotalFailed("lab1") == 1, "1 failed");
}

static void testInventionCancelJob() {
    std::cout << "\n=== Invention: CancelJob ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");
    sys.startJob("lab1", "job_1", "rifter_bp",
                 "datacore_mech", "datacore_elec");

    assertTrue(sys.cancelJob("lab1", "job_1"), "Cancel succeeds");
    assertTrue(sys.getTotalCancelled("lab1") == 1, "1 cancelled");
    assertTrue(sys.getActiveJobCount("lab1") == 0, "0 active after cancel");
    // Can't cancel again
    assertTrue(!sys.cancelJob("lab1", "job_1"), "Can't cancel again");
}

static void testInventionResearchSpeed() {
    std::cout << "\n=== Invention: ResearchSpeed ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    sys.setResearchSpeed("lab1", 2.0f);
    assertTrue(approxEqual(sys.getResearchSpeed("lab1"), 2.0f), "Speed set to 2.0");

    // Job with 100s at 2x speed should complete in 50s
    sys.startJob("lab1", "job_1", "rifter_bp",
                 "datacore_mech", "datacore_elec", 1.0f, 100.0f);
    sys.update(55.0f); // 55 * 2.0 = 110 progress
    assertTrue(sys.isJobCompleted("lab1", "job_1"), "Completed with 2x speed");
}

static void testInventionMaxConcurrent() {
    std::cout << "\n=== Invention: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    world.createEntity("lab1");
    sys.initialize("lab1");

    // Max concurrent is 3
    assertTrue(sys.startJob("lab1", "j1", "bp1", "dc1", "dc2"), "Start job 1");
    assertTrue(sys.startJob("lab1", "j2", "bp2", "dc1", "dc2"), "Start job 2");
    assertTrue(sys.startJob("lab1", "j3", "bp3", "dc1", "dc2"), "Start job 3");
    assertTrue(!sys.startJob("lab1", "j4", "bp4", "dc1", "dc2"), "Job 4 rejected (max)");
    assertTrue(sys.getActiveJobCount("lab1") == 3, "3 active jobs");
}

static void testInventionMissing() {
    std::cout << "\n=== Invention: Missing ===" << std::endl;
    ecs::World world;
    systems::InventionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.startJob("nonexistent", "j1", "bp1", "dc1", "dc2"),
               "Start fails on missing");
    assertTrue(!sys.cancelJob("nonexistent", "j1"), "Cancel fails on missing");
    assertTrue(!sys.setResearchSpeed("nonexistent", 2.0f), "SetSpeed fails on missing");
    assertTrue(sys.getJobCount("nonexistent") == 0, "0 jobs on missing");
    assertTrue(sys.getActiveJobCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalAttempted("nonexistent") == 0, "0 attempted on missing");
    assertTrue(sys.getTotalSucceeded("nonexistent") == 0, "0 succeeded on missing");
    assertTrue(sys.getTotalFailed("nonexistent") == 0, "0 failed on missing");
    assertTrue(sys.getTotalCancelled("nonexistent") == 0, "0 cancelled on missing");
    assertTrue(approxEqual(sys.getJobProgress("nonexistent", "j1"), 0.0f),
               "0 progress on missing");
    assertTrue(!sys.isJobCompleted("nonexistent", "j1"), "Not completed on missing");
    assertTrue(approxEqual(sys.getResearchSpeed("nonexistent"), 0.0f), "0 speed on missing");
}

void run_invention_system_tests() {
    testInventionCreate();
    testInventionStartJob();
    testInventionProgress();
    testInventionSuccessHighChance();
    testInventionFailLowChance();
    testInventionCancelJob();
    testInventionResearchSpeed();
    testInventionMaxConcurrent();
    testInventionMissing();
}
