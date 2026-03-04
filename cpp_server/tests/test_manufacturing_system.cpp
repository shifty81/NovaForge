// Tests for: ManufacturingSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/manufacturing_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ManufacturingSystem Tests ====================

static void testManufacturingStartJob() {
    std::cout << "\n=== Manufacturing Start Job ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station1");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_1";
    facility->station_id = "station1";
    facility->max_jobs = 2;

    auto* player = world.createEntity("player1");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player1";
    pcomp->credits = 100000.0;

    std::string job_id = mfgSys.startJob("station1", "player1", "fang_blueprint",
                                           "fang", "Fang Frigate", 1, 3600.0f, 1000.0);
    assertTrue(!job_id.empty(), "Job started successfully");
    assertTrue(mfgSys.getActiveJobCount("station1") == 1, "1 active job");
    assertTrue(approxEqual(static_cast<float>(pcomp->credits), 99000.0f), "Install cost deducted");
}

static void testManufacturingJobCompletion() {
    std::cout << "\n=== Manufacturing Job Completion ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station2");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_2";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player2");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player2";
    pcomp->credits = 100000.0;

    mfgSys.startJob("station2", "player2", "autocannon_bp",
                     "autocannon_i", "150mm Autocannon I", 1, 100.0f, 500.0);

    assertTrue(mfgSys.getActiveJobCount("station2") == 1, "Job is active");
    assertTrue(mfgSys.getCompletedJobCount("station2") == 0, "No completed jobs yet");

    // Tick to completion
    mfgSys.update(100.0f);
    assertTrue(mfgSys.getActiveJobCount("station2") == 0, "No active jobs after completion");
    assertTrue(mfgSys.getCompletedJobCount("station2") == 1, "1 completed job");
    assertTrue(mfgSys.getTotalRunsCompleted("station2") == 1, "1 run completed");
}

static void testManufacturingMultipleRuns() {
    std::cout << "\n=== Manufacturing Multiple Runs ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station3");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_3";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player3");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player3";
    pcomp->credits = 100000.0;

    mfgSys.startJob("station3", "player3", "drone_bp",
                     "hobgoblin_i", "Hobgoblin I", 3, 50.0f, 200.0);

    // First run
    mfgSys.update(50.0f);
    assertTrue(mfgSys.getTotalRunsCompleted("station3") == 1, "1 run after 50s");
    assertTrue(mfgSys.getActiveJobCount("station3") == 1, "Job still active (more runs)");

    // Second run
    mfgSys.update(50.0f);
    assertTrue(mfgSys.getTotalRunsCompleted("station3") == 2, "2 runs after 100s");

    // Third run (final)
    mfgSys.update(50.0f);
    assertTrue(mfgSys.getTotalRunsCompleted("station3") == 3, "3 runs after 150s");
    assertTrue(mfgSys.getCompletedJobCount("station3") == 1, "Job completed");
    assertTrue(mfgSys.getActiveJobCount("station3") == 0, "No active jobs");
}

static void testManufacturingJobSlotLimit() {
    std::cout << "\n=== Manufacturing Job Slot Limit ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station4");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_4";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player4");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player4";
    pcomp->credits = 100000.0;

    std::string job1 = mfgSys.startJob("station4", "player4", "bp1",
                                         "item1", "Item 1", 1, 3600.0f, 100.0);
    assertTrue(!job1.empty(), "First job started");

    std::string job2 = mfgSys.startJob("station4", "player4", "bp2",
                                         "item2", "Item 2", 1, 3600.0f, 100.0);
    assertTrue(job2.empty(), "Second job rejected (slot full)");
    assertTrue(mfgSys.getActiveJobCount("station4") == 1, "Still 1 active job");
}

static void testManufacturingCancelJob() {
    std::cout << "\n=== Manufacturing Cancel Job ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station5");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_5";
    facility->max_jobs = 2;

    auto* player = world.createEntity("player5");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player5";
    pcomp->credits = 100000.0;

    std::string job_id = mfgSys.startJob("station5", "player5", "bp_test",
                                           "item_test", "Test Item", 1, 3600.0f, 100.0);
    assertTrue(mfgSys.getActiveJobCount("station5") == 1, "1 active job");

    bool cancelled = mfgSys.cancelJob("station5", job_id);
    assertTrue(cancelled, "Job cancelled successfully");
    assertTrue(mfgSys.getActiveJobCount("station5") == 0, "No active jobs after cancel");
}

static void testManufacturingInsufficientFunds() {
    std::cout << "\n=== Manufacturing Insufficient Funds ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station6");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_6";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player6");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player6";
    pcomp->credits = 50.0;  // Not enough

    std::string job_id = mfgSys.startJob("station6", "player6", "bp_expensive",
                                           "item_expensive", "Expensive Item", 1, 3600.0f, 1000.0);
    assertTrue(job_id.empty(), "Job rejected (insufficient funds)");
    assertTrue(mfgSys.getActiveJobCount("station6") == 0, "No active jobs");
    assertTrue(approxEqual(static_cast<float>(pcomp->credits), 50.0f), "Credits unchanged");
}


void run_manufacturing_system_tests() {
    testManufacturingStartJob();
    testManufacturingJobCompletion();
    testManufacturingMultipleRuns();
    testManufacturingJobSlotLimit();
    testManufacturingCancelJob();
    testManufacturingInsufficientFunds();
}
