// Tests for: SalvageProcessingSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/salvage_processing_system.h"

using namespace atlas;

// ==================== SalvageProcessingSystem Tests ====================

static void testSalvageCreate() {
    std::cout << "\n=== SalvageProcessing: Create ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    assertTrue(sys.initialize("salvager1", 2.0f), "Init succeeds");
    assertTrue(sys.getJobCount("salvager1") == 0, "Zero jobs");
    assertTrue(sys.getActiveJobCount("salvager1") == 0, "Zero active");
    assertTrue(approxEqual(sys.getProcessingSpeed("salvager1"), 2.0f), "Speed set");
    assertTrue(approxEqual(sys.getSkillBonus("salvager1"), 0.0f), "Zero skill bonus");
    assertTrue(approxEqual(sys.getTotalMaterialsSalvaged("salvager1"), 0.0f), "Zero materials");
    assertTrue(sys.getTotalJobsCompleted("salvager1") == 0, "Zero completed");
    assertTrue(sys.getTotalJobsFailed("salvager1") == 0, "Zero failed");
}

static void testSalvageInvalidInit() {
    std::cout << "\n=== SalvageProcessing: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    assertTrue(!sys.initialize("missing"), "Missing entity fails");
    world.createEntity("s1");
    assertTrue(!sys.initialize("s1", 0.0f), "Zero speed rejected");
    assertTrue(!sys.initialize("s1", -1.0f), "Negative speed rejected");
}

static void testSalvageAddJob() {
    std::cout << "\n=== SalvageProcessing: AddJob ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1");

    assertTrue(sys.addJob("salvager1", "wreck_a", "metal", 30.0f, 100.0f, 0.8f), "Add job A");
    assertTrue(sys.addJob("salvager1", "wreck_b", "alloy", 20.0f, 50.0f, 0.6f), "Add job B");
    assertTrue(sys.getJobCount("salvager1") == 2, "2 jobs");
    assertTrue(sys.getActiveJobCount("salvager1") == 2, "2 active");

    assertTrue(!sys.addJob("salvager1", "wreck_a", "scrap", 10.0f, 20.0f, 0.5f), "Duplicate rejected");
    assertTrue(!sys.addJob("salvager1", "", "metal", 30.0f, 100.0f, 0.8f), "Empty ID rejected");
    assertTrue(!sys.addJob("salvager1", "x", "", 30.0f, 100.0f, 0.8f), "Empty type rejected");
    assertTrue(!sys.addJob("salvager1", "x", "metal", 0.0f, 100.0f, 0.8f), "Zero time rejected");
    assertTrue(!sys.addJob("salvager1", "x", "metal", 30.0f, 0.0f, 0.8f), "Zero yield rejected");
    assertTrue(!sys.addJob("salvager1", "x", "metal", 30.0f, 100.0f, -0.1f), "Negative chance rejected");
    assertTrue(!sys.addJob("salvager1", "x", "metal", 30.0f, 100.0f, 1.1f), "Chance > 1 rejected");
    assertTrue(!sys.addJob("nonexistent", "x", "metal", 30.0f, 100.0f, 0.8f), "Missing entity rejected");
}

static void testSalvageRemoveJob() {
    std::cout << "\n=== SalvageProcessing: RemoveJob ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1");
    sys.addJob("salvager1", "wreck_a", "metal", 30.0f, 100.0f, 0.8f);
    sys.addJob("salvager1", "wreck_b", "alloy", 20.0f, 50.0f, 0.6f);

    assertTrue(sys.removeJob("salvager1", "wreck_a"), "Remove wreck_a succeeds");
    assertTrue(sys.getJobCount("salvager1") == 1, "1 job remaining");
    assertTrue(!sys.removeJob("salvager1", "wreck_a"), "Double remove fails");
    assertTrue(!sys.removeJob("salvager1", "nonexistent"), "Remove nonexistent fails");
}

static void testSalvageProcessing() {
    std::cout << "\n=== SalvageProcessing: Processing ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 1.0f);
    // processing_time=10, so at speed 1.0 it takes 10s
    sys.addJob("salvager1", "wreck_a", "metal", 10.0f, 50.0f, 0.8f);

    sys.update(5.0f);
    float progress = sys.getJobProgress("salvager1", "wreck_a");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% progress at 5s");
    assertTrue(!sys.isJobCompleted("salvager1", "wreck_a"), "Not completed yet");

    sys.update(5.0f);
    assertTrue(sys.isJobCompleted("salvager1", "wreck_a"), "Completed at 10s");
    // success_chance(0.8) + skill_bonus(0) = 0.8 >= 0.5 → success
    assertTrue(sys.isJobSuccessful("salvager1", "wreck_a"), "Job successful");
    assertTrue(sys.getTotalJobsCompleted("salvager1") == 1, "1 completed");
    float salvaged = sys.getTotalMaterialsSalvaged("salvager1");
    assertTrue(salvaged > 49.0f && salvaged < 51.0f, "~50 materials salvaged");
}

static void testSalvageFailure() {
    std::cout << "\n=== SalvageProcessing: Failure ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 1.0f);
    // success_chance=0.3 + skill_bonus(0) = 0.3 < 0.5 → fail
    sys.addJob("salvager1", "wreck_a", "junk", 5.0f, 20.0f, 0.3f);

    sys.update(5.0f);
    assertTrue(sys.isJobCompleted("salvager1", "wreck_a"), "Completed");
    assertTrue(!sys.isJobSuccessful("salvager1", "wreck_a"), "Job failed");
    assertTrue(sys.getTotalJobsFailed("salvager1") == 1, "1 failed");
    assertTrue(approxEqual(sys.getTotalMaterialsSalvaged("salvager1"), 0.0f), "No materials from failure");
}

static void testSalvageInvalidOperations() {
    std::cout << "\n=== SalvageProcessing: InvalidOperations ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1");

    assertTrue(!sys.setProcessingSpeed("salvager1", 0.0f), "Zero speed rejected");
    assertTrue(!sys.setProcessingSpeed("salvager1", -1.0f), "Negative speed rejected");
    assertTrue(!sys.setSkillBonus("salvager1", -0.1f), "Negative bonus rejected");
    assertTrue(!sys.setProcessingSpeed("missing", 1.0f), "Missing entity rejected");
    assertTrue(!sys.setSkillBonus("missing", 0.1f), "Missing entity rejected");
}

static void testSalvageUpdateTick() {
    std::cout << "\n=== SalvageProcessing: UpdateTick ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 2.0f);
    sys.setSkillBonus("salvager1", 1.0f);
    // speed(2) + bonus(1) = 3 effective, time = 30
    // progress per second = 3/30 = 0.1
    sys.addJob("salvager1", "wreck_a", "metal", 30.0f, 100.0f, 0.8f);

    sys.update(5.0f); // progress = 0.5
    float progress = sys.getJobProgress("salvager1", "wreck_a");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% at 5s with bonuses");
}

static void testSalvageMultipleJobs() {
    std::cout << "\n=== SalvageProcessing: MultipleJobs ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 1.0f);

    for (int i = 0; i < 5; i++) {
        std::string id = "wreck_" + std::to_string(i);
        sys.addJob("salvager1", id, "metal", 10.0f, 20.0f, 0.8f);
    }
    assertTrue(sys.getJobCount("salvager1") == 5, "5 jobs");
    assertTrue(sys.getActiveJobCount("salvager1") == 5, "5 active");

    sys.update(10.0f); // all complete
    assertTrue(sys.getActiveJobCount("salvager1") == 0, "0 active after completion");
    assertTrue(sys.getTotalJobsCompleted("salvager1") == 5, "5 completed");
}

static void testSalvageBoundary() {
    std::cout << "\n=== SalvageProcessing: Boundary ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 100.0f);
    // Very fast processing
    sys.addJob("salvager1", "wreck_a", "metal", 1.0f, 10.0f, 1.0f);

    sys.update(0.01f); // should complete instantly at 100x speed
    assertTrue(sys.isJobCompleted("salvager1", "wreck_a"), "Completes with high speed");

    // Skill bonus tips a failing job to success
    sys.addJob("salvager1", "wreck_b", "scrap", 1.0f, 5.0f, 0.3f);
    sys.setSkillBonus("salvager1", 0.2f); // 0.3 + 0.2 = 0.5 >= 0.5 → success
    sys.update(1.0f);
    assertTrue(sys.isJobSuccessful("salvager1", "wreck_b"), "Skill bonus tips to success");
}

static void testSalvageMissing() {
    std::cout << "\n=== SalvageProcessing: Missing ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    assertTrue(sys.getJobCount("x") == 0, "Default jobs on missing");
    assertTrue(sys.getActiveJobCount("x") == 0, "Default active on missing");
    assertTrue(approxEqual(sys.getJobProgress("x", "w"), 0.0f), "Default progress on missing");
    assertTrue(!sys.isJobCompleted("x", "w"), "Default completed on missing");
    assertTrue(!sys.isJobSuccessful("x", "w"), "Default successful on missing");
    assertTrue(approxEqual(sys.getTotalMaterialsSalvaged("x"), 0.0f), "Default materials on missing");
    assertTrue(sys.getTotalJobsCompleted("x") == 0, "Default completed count on missing");
    assertTrue(sys.getTotalJobsFailed("x") == 0, "Default failed count on missing");
    assertTrue(approxEqual(sys.getProcessingSpeed("x"), 0.0f), "Default speed on missing");
    assertTrue(approxEqual(sys.getSkillBonus("x"), 0.0f), "Default bonus on missing");
}

static void testSalvageStats() {
    std::cout << "\n=== SalvageProcessing: Stats ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 1.0f);
    sys.addJob("salvager1", "w1", "metal", 5.0f, 30.0f, 0.8f);  // success
    sys.addJob("salvager1", "w2", "junk", 5.0f, 10.0f, 0.3f);   // fail

    sys.update(5.0f);
    assertTrue(sys.getTotalJobsCompleted("salvager1") == 1, "1 successful job");
    assertTrue(sys.getTotalJobsFailed("salvager1") == 1, "1 failed job");
    float salvaged = sys.getTotalMaterialsSalvaged("salvager1");
    assertTrue(salvaged > 29.0f && salvaged < 31.0f, "~30 materials from success only");
}

static void testSalvageCombined() {
    std::cout << "\n=== SalvageProcessing: Combined ===" << std::endl;
    ecs::World world;
    systems::SalvageProcessingSystem sys(&world);
    world.createEntity("salvager1");
    sys.initialize("salvager1", 1.0f);

    sys.addJob("salvager1", "w1", "metal", 10.0f, 50.0f, 0.8f);
    sys.addJob("salvager1", "w2", "alloy", 20.0f, 100.0f, 0.7f);
    sys.setProcessingSpeed("salvager1", 2.0f);

    // After 5s: w1 progress = 2*5/10 = 1.0 (done), w2 = 2*5/20 = 0.5
    sys.update(5.0f);
    assertTrue(sys.isJobCompleted("salvager1", "w1"), "w1 completed");
    assertTrue(!sys.isJobCompleted("salvager1", "w2"), "w2 not yet");
    assertTrue(sys.getActiveJobCount("salvager1") == 1, "1 active remaining");

    sys.removeJob("salvager1", "w2");
    assertTrue(sys.getJobCount("salvager1") == 1, "1 job after remove");
    assertTrue(approxEqual(sys.getProcessingSpeed("salvager1"), 2.0f), "Speed unchanged");
}

void run_salvage_processing_system_tests() {
    testSalvageCreate();
    testSalvageInvalidInit();
    testSalvageAddJob();
    testSalvageRemoveJob();
    testSalvageProcessing();
    testSalvageFailure();
    testSalvageInvalidOperations();
    testSalvageUpdateTick();
    testSalvageMultipleJobs();
    testSalvageBoundary();
    testSalvageMissing();
    testSalvageStats();
    testSalvageCombined();
}
