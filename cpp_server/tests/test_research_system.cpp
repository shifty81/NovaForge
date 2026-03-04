// Tests for: ResearchSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/research_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== ResearchSystem Tests ====================

static void testResearchME() {
    std::cout << "\n=== Research ME ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab1");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_1";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher1");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher1";
    pcomp->credits = 100000.0;

    std::string job_id = resSys.startMEResearch("lab1", "researcher1", "fang_blueprint",
                                                  5, 100.0f, 500.0);
    assertTrue(!job_id.empty(), "ME research started");
    assertTrue(resSys.getActiveJobCount("lab1") == 1, "1 active job");
    assertTrue(approxEqual(static_cast<float>(pcomp->credits), 99500.0f), "Install cost deducted");

    // Complete
    resSys.update(100.0f);
    assertTrue(resSys.getActiveJobCount("lab1") == 0, "No active jobs");
    assertTrue(resSys.getCompletedJobCount("lab1") == 1, "1 completed job");
}

static void testResearchTE() {
    std::cout << "\n=== Research TE ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab2");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_2";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher2");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher2";
    pcomp->credits = 100000.0;

    std::string job_id = resSys.startTEResearch("lab2", "researcher2", "autocannon_bp",
                                                  10, 200.0f, 300.0);
    assertTrue(!job_id.empty(), "TE research started");
    assertTrue(resSys.getActiveJobCount("lab2") == 1, "1 active job");

    resSys.update(200.0f);
    assertTrue(resSys.getCompletedJobCount("lab2") == 1, "TE research completed");
}

static void testResearchInvention() {
    std::cout << "\n=== Research Invention ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab3");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_3";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher3");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher3";
    pcomp->credits = 100000.0;

    std::string job_id = resSys.startInvention("lab3", "researcher3",
                                                "fang_blueprint", "fang_ii_blueprint",
                                                "datacore_mechanical_engineering",
                                                "datacore_electronic_engineering",
                                                1.0f, // 100% success for testing
                                                50.0f, 1000.0);
    assertTrue(!job_id.empty(), "Invention started");
    assertTrue(resSys.getActiveJobCount("lab3") == 1, "1 active job");

    resSys.update(50.0f);
    // With 100% success chance, it should complete
    assertTrue(resSys.getCompletedJobCount("lab3") == 1, "Invention succeeded");
    assertTrue(resSys.getFailedJobCount("lab3") == 0, "No failed jobs");
}

static void testResearchInventionFailure() {
    std::cout << "\n=== Research Invention Failure ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab4");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_4";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher4");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher4";
    pcomp->credits = 100000.0;

    std::string job_id = resSys.startInvention("lab4", "researcher4",
                                                "fang_blueprint", "fang_ii_blueprint",
                                                "datacore_mechanical_engineering",
                                                "datacore_electronic_engineering",
                                                0.0f, // 0% success = guaranteed fail
                                                50.0f, 500.0);
    assertTrue(!job_id.empty(), "Invention job started");

    resSys.update(50.0f);
    assertTrue(resSys.getFailedJobCount("lab4") == 1, "Invention failed (0% chance)");
    assertTrue(resSys.getCompletedJobCount("lab4") == 0, "No completed jobs");
}

static void testResearchJobSlotLimit() {
    std::cout << "\n=== Research Job Slot Limit ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab5");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_5";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher5");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher5";
    pcomp->credits = 100000.0;

    std::string job1 = resSys.startMEResearch("lab5", "researcher5", "bp1",
                                                5, 1000.0f, 100.0);
    assertTrue(!job1.empty(), "First research job started");

    std::string job2 = resSys.startTEResearch("lab5", "researcher5", "bp2",
                                                10, 1000.0f, 100.0);
    assertTrue(job2.empty(), "Second job rejected (slot full)");
    assertTrue(resSys.getActiveJobCount("lab5") == 1, "Still 1 active job");
}

static void testResearchInsufficientFunds() {
    std::cout << "\n=== Research Insufficient Funds ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab6");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_6";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher6");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher6";
    pcomp->credits = 10.0;  // Not enough

    std::string job_id = resSys.startMEResearch("lab6", "researcher6", "bp_expensive",
                                                  5, 1000.0f, 500.0);
    assertTrue(job_id.empty(), "Job rejected (insufficient funds)");
    assertTrue(resSys.getActiveJobCount("lab6") == 0, "No active jobs");
    assertTrue(approxEqual(static_cast<float>(pcomp->credits), 10.0f), "Credits unchanged");
}


void run_research_system_tests() {
    testResearchME();
    testResearchTE();
    testResearchInvention();
    testResearchInventionFailure();
    testResearchJobSlotLimit();
    testResearchInsufficientFunds();
}
