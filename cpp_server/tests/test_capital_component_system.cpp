// Tests for: CapitalComponentSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/capital_component_system.h"

using namespace atlas;

// ==================== CapitalComponentSystem Tests ====================

static void testCapCompInit() {
    std::cout << "\n=== CapComp: Init ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    assertTrue(sys.initialize("fac1", "facility_001"), "Init succeeds");
    assertTrue(sys.getActiveJobCount("fac1") == 0, "0 active jobs");
    assertTrue(sys.getCompletedJobCount("fac1") == 0, "0 completed jobs");
    assertTrue(sys.getTotalUnitsProduced("fac1") == 0, "0 units produced");
}

static void testCapCompStartJob() {
    std::cout << "\n=== CapComp: StartJob ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    std::string jid = sys.startJob("fac1", "cap_armor_plate", "bp_001", 1,
                                   0.0f, 0.0f);
    assertTrue(!jid.empty(), "Job id returned");
    assertTrue(sys.getActiveJobCount("fac1") == 1, "1 active job");
}

static void testCapCompStartZeroRuns() {
    std::cout << "\n=== CapComp: StartZeroRuns ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    std::string jid = sys.startJob("fac1", "cap_armor_plate", "bp_001", 0,
                                   0.0f, 0.0f);
    assertTrue(jid.empty(), "0 runs rejected");
    assertTrue(sys.getActiveJobCount("fac1") == 0, "0 active jobs");
}

static void testCapCompMaxConcurrent() {
    std::cout << "\n=== CapComp: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    sys.startJob("fac1", "cap_armor_plate", "bp_001", 1, 0.0f, 0.0f);
    sys.startJob("fac1", "cap_shield_emitter", "bp_002", 1, 0.0f, 0.0f);
    assertTrue(sys.getActiveJobCount("fac1") == 2, "2 jobs at limit");

    std::string jid = sys.startJob("fac1", "cap_cap_battery", "bp_003", 1,
                                    0.0f, 0.0f);
    assertTrue(jid.empty(), "3rd job rejected");
    assertTrue(sys.getActiveJobCount("fac1") == 2, "Still 2 active");
}

static void testCapCompProgress() {
    std::cout << "\n=== CapComp: Progress ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    std::string jid = sys.startJob("fac1", "cap_armor_plate", "bp_001", 1,
                                   0.0f, 0.0f);
    sys.update(3600.0f);  // 1 hour (default 8h job)
    float prog = sys.getJobProgress("fac1", jid);
    assertTrue(prog > 0.0f && prog < 1.0f, "Progress between 0 and 1");
    assertTrue(!sys.isJobComplete("fac1", jid), "Not complete yet");
}

static void testCapCompComplete() {
    std::cout << "\n=== CapComp: Complete ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    std::string jid = sys.startJob("fac1", "cap_armor_plate", "bp_001", 3,
                                   0.0f, 0.0f);
    sys.update(200000.0f);  // well past 8h * 3 runs
    assertTrue(sys.isJobComplete("fac1", jid), "Job complete");
    assertTrue(sys.getCompletedJobCount("fac1") == 1, "1 completed");
    assertTrue(sys.getTotalUnitsProduced("fac1") == 3, "3 units produced");
    assertTrue(sys.getActiveJobCount("fac1") == 0, "0 active");
}

static void testCapCompTeBonus() {
    std::cout << "\n=== CapComp: TeBonus ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    // 20% TE bonus → effective time = 8h * 0.8 = 6.4h
    std::string jid = sys.startJob("fac1", "cap_armor_plate", "bp_001", 1,
                                   0.0f, 0.2f);
    float target = 28800.0f * 0.8f;  // 23040s
    sys.update(target - 1.0f);
    assertTrue(!sys.isJobComplete("fac1", jid), "Not done before TE time");
    sys.update(5.0f);
    assertTrue(sys.isJobComplete("fac1", jid), "Done after TE time");
}

static void testCapCompCancelJob() {
    std::cout << "\n=== CapComp: CancelJob ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    std::string jid = sys.startJob("fac1", "cap_armor_plate", "bp_001", 1,
                                   0.0f, 0.0f);
    assertTrue(sys.cancelJob("fac1", jid), "Cancel succeeds");
    assertTrue(sys.getActiveJobCount("fac1") == 0, "0 active after cancel");
    assertTrue(!sys.cancelJob("fac1", jid), "Cancel again fails");
}

static void testCapCompSlotsFreedAfterComplete() {
    std::cout << "\n=== CapComp: SlotsFreedAfterComplete ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    world.createEntity("fac1");
    sys.initialize("fac1", "facility_001");

    sys.startJob("fac1", "cap_armor_plate", "bp_001", 1, 0.0f, 0.0f);
    sys.startJob("fac1", "cap_shield_emitter", "bp_002", 1, 0.0f, 0.0f);
    sys.update(200000.0f);  // complete both

    assertTrue(sys.getActiveJobCount("fac1") == 0, "Both complete");
    // Now a new job should be accepted
    std::string jid = sys.startJob("fac1", "cap_cap_battery", "bp_003", 1,
                                    0.0f, 0.0f);
    assertTrue(!jid.empty(), "New job accepted after slots freed");
}

static void testCapCompMissing() {
    std::cout << "\n=== CapComp: Missing ===" << std::endl;
    ecs::World world;
    systems::CapitalComponentSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "fac_x"),
               "Init fails on missing entity");
    std::string jid = sys.startJob("nonexistent", "cap_armor_plate",
                                    "bp_x", 1, 0.0f, 0.0f);
    assertTrue(jid.empty(), "StartJob fails on missing");
    assertTrue(!sys.cancelJob("nonexistent", "capjob_1"),
               "Cancel fails on missing");
    assertTrue(sys.getActiveJobCount("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getCompletedJobCount("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getTotalUnitsProduced("nonexistent") == 0, "0 on missing");
    assertTrue(approxEqual(sys.getJobProgress("nonexistent", "j1"), 0.0f),
               "0 progress on missing");
    assertTrue(!sys.isJobComplete("nonexistent", "j1"),
               "Not complete on missing");
}

void run_capital_component_system_tests() {
    testCapCompInit();
    testCapCompStartJob();
    testCapCompStartZeroRuns();
    testCapCompMaxConcurrent();
    testCapCompProgress();
    testCapCompComplete();
    testCapCompTeBonus();
    testCapCompCancelJob();
    testCapCompSlotsFreedAfterComplete();
    testCapCompMissing();
}
