// Tests for: RepairTimerSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/repair_timer_system.h"

using namespace atlas;
using RepairLayer = components::RepairTimerState::RepairLayer;

static void testRepairTimerInit() {
    std::cout << "\n=== RepairTimer: Init ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(sys.getJobCount("e1") == 0, "No jobs after init");
    assertTrue(sys.getTotalRepaired("e1") == 0.0f, "No repaired after init");
    assertTrue(sys.getTotalJobsStarted("e1") == 0, "No jobs started after init");
    assertTrue(sys.getTotalJobsCompleted("e1") == 0, "No jobs completed after init");
}

static void testRepairTimerAddJob() {
    std::cout << "\n=== RepairTimer: AddJob ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addJob("e1", "j1", RepairLayer::Hull, 50.0f, 5), "Add hull job");
    assertTrue(sys.getJobCount("e1") == 1, "Job count is 1");
    assertTrue(sys.hasJob("e1", "j1"), "Has job j1");
    assertTrue(sys.isJobActive("e1", "j1"), "Job j1 is active");
    assertTrue(!sys.isJobComplete("e1", "j1"), "Job j1 not complete");
    assertTrue(sys.getTicksRemaining("e1", "j1") == 5, "5 ticks remaining");
    assertTrue(sys.getTotalJobsStarted("e1") == 1, "1 job started");

    // Duplicate rejected
    assertTrue(!sys.addJob("e1", "j1", RepairLayer::Armor, 10.0f, 3), "Duplicate rejected");

    // Invalid args
    assertTrue(!sys.addJob("e1", "", RepairLayer::Hull, 10.0f, 3), "Empty job_id rejected");
    assertTrue(!sys.addJob("e1", "j2", RepairLayer::Hull, 0.0f, 3), "Zero amount rejected");
    assertTrue(!sys.addJob("e1", "j2", RepairLayer::Hull, -1.0f, 3), "Negative amount rejected");
    assertTrue(!sys.addJob("e1", "j2", RepairLayer::Hull, 10.0f, 0), "Zero ticks rejected");
    assertTrue(!sys.addJob("e1", "j2", RepairLayer::Hull, 10.0f, -1), "Negative ticks rejected");

    assertTrue(sys.getJobCount("e1") == 1, "Still 1 job after invalid adds");
}

static void testRepairTimerUpdate() {
    std::cout << "\n=== RepairTimer: Update ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Hull, 100.0f, 3);

    sys.update(1.0f);
    assertTrue(sys.getTicksRemaining("e1", "j1") == 2, "2 ticks after 1 update");
    assertTrue(approxEqual(sys.getTotalRepaired("e1"), 100.0f), "100 repaired after 1 tick");
    assertTrue(sys.isJobActive("e1", "j1"), "Still active after 1 tick");

    sys.update(1.0f);
    assertTrue(sys.getTicksRemaining("e1", "j1") == 1, "1 tick remaining");
    assertTrue(approxEqual(sys.getTotalRepaired("e1"), 200.0f), "200 repaired after 2 ticks");

    sys.update(1.0f);
    assertTrue(sys.getTicksRemaining("e1", "j1") == 0, "0 ticks remaining");
    assertTrue(approxEqual(sys.getTotalRepaired("e1"), 300.0f), "300 repaired after 3 ticks");
    assertTrue(sys.isJobComplete("e1", "j1"), "Job complete after 3 ticks");
    assertTrue(!sys.isJobActive("e1", "j1"), "Not active after completion");
    assertTrue(sys.getTotalJobsCompleted("e1") == 1, "1 job completed");
}

static void testRepairTimerCancelJob() {
    std::cout << "\n=== RepairTimer: CancelJob ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Armor, 50.0f, 5);
    sys.addJob("e1", "j2", RepairLayer::Shield, 30.0f, 3);

    assertTrue(sys.getJobCount("e1") == 2, "2 jobs before cancel");
    assertTrue(sys.cancelJob("e1", "j1"), "Cancel j1 succeeds");
    assertTrue(sys.getJobCount("e1") == 1, "1 job after cancel");
    assertTrue(!sys.hasJob("e1", "j1"), "j1 removed");
    assertTrue(sys.hasJob("e1", "j2"), "j2 still present");
    assertTrue(!sys.cancelJob("e1", "j1"), "Cancel missing job fails");
    assertTrue(!sys.cancelJob("nonexistent", "j2"), "Cancel on missing entity fails");
}

static void testRepairTimerClearJobs() {
    std::cout << "\n=== RepairTimer: ClearJobs ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Hull, 50.0f, 5);
    sys.addJob("e1", "j2", RepairLayer::Armor, 30.0f, 3);

    assertTrue(sys.getJobCount("e1") == 2, "2 jobs before clear");
    assertTrue(sys.clearJobs("e1"), "ClearJobs succeeds");
    assertTrue(sys.getJobCount("e1") == 0, "0 jobs after clear");
    assertTrue(!sys.clearJobs("nonexistent"), "ClearJobs on missing entity fails");
}

static void testRepairTimerMaxCap() {
    std::cout << "\n=== RepairTimer: MaxCap ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    for (int i = 0; i < 10; i++) {
        std::string id = "j" + std::to_string(i);
        assertTrue(sys.addJob("e1", id, RepairLayer::Hull, 10.0f, 5), "Add job " + id);
    }
    assertTrue(sys.getJobCount("e1") == 10, "10 jobs at cap");
    assertTrue(!sys.addJob("e1", "j_over", RepairLayer::Hull, 10.0f, 5), "11th job rejected");
}

static void testRepairTimerMultipleJobs() {
    std::cout << "\n=== RepairTimer: MultipleJobs ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Hull,   50.0f, 2);
    sys.addJob("e1", "j2", RepairLayer::Armor,  30.0f, 4);
    sys.addJob("e1", "j3", RepairLayer::Shield, 20.0f, 3);

    sys.update(1.0f);
    assertTrue(sys.getActiveJobCount("e1") == 3, "3 active after 1 tick");
    assertTrue(approxEqual(sys.getTotalRepaired("e1"), 100.0f), "100 repaired (50+30+20)");

    sys.update(1.0f);
    // j1 completes (2 ticks done)
    assertTrue(sys.isJobComplete("e1", "j1"), "j1 complete after 2 ticks");
    assertTrue(sys.getActiveJobCount("e1") == 2, "2 active after j1 completes");
    assertTrue(approxEqual(sys.getTotalRepaired("e1"), 200.0f), "200 repaired after 2 ticks");
}

static void testRepairTimerGetRepairByLayer() {
    std::cout << "\n=== RepairTimer: GetRepairByLayer ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Hull,   100.0f, 3);
    sys.addJob("e1", "j2", RepairLayer::Armor,   50.0f, 2);

    // No ticks yet
    assertTrue(approxEqual(sys.getRepairByLayer("e1", RepairLayer::Hull), 0.0f), "Hull layer 0 before ticks");
    assertTrue(approxEqual(sys.getRepairByLayer("e1", RepairLayer::Armor), 0.0f), "Armor layer 0 before ticks");

    sys.update(1.0f);
    assertTrue(approxEqual(sys.getRepairByLayer("e1", RepairLayer::Hull), 100.0f), "Hull layer 100 after 1 tick");
    assertTrue(approxEqual(sys.getRepairByLayer("e1", RepairLayer::Armor), 50.0f), "Armor layer 50 after 1 tick");
    assertTrue(approxEqual(sys.getRepairByLayer("e1", RepairLayer::Shield), 0.0f), "Shield layer 0 (no shield job)");

    // Missing entity
    assertTrue(approxEqual(sys.getRepairByLayer("nonexistent", RepairLayer::Hull), 0.0f), "Missing entity 0");
}

static void testRepairTimerInactiveSkipsUpdate() {
    std::cout << "\n=== RepairTimer: InactiveSkips ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Hull, 50.0f, 3);

    auto* comp = world.getEntity("e1")->getComponent<components::RepairTimerState>();
    comp->active = false;
    sys.update(1.0f);
    assertTrue(sys.getTotalRepaired("e1") == 0.0f, "No repair when inactive");
    assertTrue(sys.getTicksRemaining("e1", "j1") == 3, "Ticks unchanged when inactive");
}

static void testRepairTimerMissing() {
    std::cout << "\n=== RepairTimer: Missing entity ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    const std::string missing = "nonexistent";

    assertTrue(!sys.addJob(missing, "j1", RepairLayer::Hull, 10.0f, 5), "addJob missing entity");
    assertTrue(!sys.cancelJob(missing, "j1"), "cancelJob missing entity");
    assertTrue(!sys.clearJobs(missing), "clearJobs missing entity");
    assertTrue(sys.getJobCount(missing) == 0, "getJobCount missing entity");
    assertTrue(sys.getActiveJobCount(missing) == 0, "getActiveJobCount missing entity");
    assertTrue(!sys.isJobActive(missing, "j1"), "isJobActive missing entity");
    assertTrue(!sys.isJobComplete(missing, "j1"), "isJobComplete missing entity");
    assertTrue(!sys.hasJob(missing, "j1"), "hasJob missing entity");
    assertTrue(sys.getTicksRemaining(missing, "j1") == 0, "getTicksRemaining missing entity");
    assertTrue(approxEqual(sys.getTotalRepaired(missing), 0.0f), "getTotalRepaired missing entity");
    assertTrue(sys.getTotalJobsStarted(missing) == 0, "getTotalJobsStarted missing entity");
    assertTrue(sys.getTotalJobsCompleted(missing) == 0, "getTotalJobsCompleted missing entity");
    assertTrue(approxEqual(sys.getRepairByLayer(missing, RepairLayer::Hull), 0.0f), "getRepairByLayer missing entity");
}

static void testRepairTimerCompletedJobNotDoubleProcessed() {
    std::cout << "\n=== RepairTimer: CompletedNotDoubleProcessed ===" << std::endl;
    ecs::World world;
    systems::RepairTimerSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addJob("e1", "j1", RepairLayer::Hull, 50.0f, 2);

    sys.update(1.0f);
    sys.update(1.0f);
    assertTrue(sys.isJobComplete("e1", "j1"), "Job j1 complete");
    float repaired_after_complete = sys.getTotalRepaired("e1");

    // More ticks should not add to total_repaired for completed job
    sys.update(1.0f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getTotalRepaired("e1"), repaired_after_complete), "No extra repair after completion");
    assertTrue(sys.getTotalJobsCompleted("e1") == 1, "Still 1 completed");
}

void run_repair_timer_system_tests() {
    testRepairTimerInit();
    testRepairTimerAddJob();
    testRepairTimerUpdate();
    testRepairTimerCancelJob();
    testRepairTimerClearJobs();
    testRepairTimerMaxCap();
    testRepairTimerMultipleJobs();
    testRepairTimerGetRepairByLayer();
    testRepairTimerInactiveSkipsUpdate();
    testRepairTimerMissing();
    testRepairTimerCompletedJobNotDoubleProcessed();
}
