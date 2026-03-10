// Tests for: FleetAfterActionReportSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_after_action_report_system.h"

using namespace atlas;

using FAAR  = components::FleetAfterActionReport;

// ==================== FleetAfterActionReportSystem Tests ====================

static void testFAARInit() {
    std::cout << "\n=== FAAR: Init ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initialize("fleet1"), "Init succeeds");
    assertTrue(sys.getState("fleet1") == FAAR::State::Idle,
               "Initial state is Idle");
    assertTrue(sys.getMemberCount("fleet1") == 0, "Zero members initially");
    assertTrue(sys.getTotalKills("fleet1")   == 0, "Zero kills initially");
    assertTrue(sys.getTotalLosses("fleet1")  == 0, "Zero losses initially");
    assertTrue(approxEqual(sys.getTotalDamage("fleet1"), 0.0f), "Zero damage initially");
    assertTrue(approxEqual(sys.getTotalLoot("fleet1"),   0.0f), "Zero loot initially");
    assertTrue(sys.getTotalReports("fleet1") == 0, "Zero reports initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFAARStartReport() {
    std::cout << "\n=== FAAR: StartReport ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");

    assertTrue(sys.startReport("fleet1"), "StartReport succeeds from Idle");
    assertTrue(sys.getState("fleet1") == FAAR::State::Recording,
               "State is Recording after start");
    assertTrue(!sys.startReport("nonexistent"), "StartReport fails on missing entity");
}

static void testFAARAddMember() {
    std::cout << "\n=== FAAR: AddMember ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");
    sys.startReport("fleet1");

    assertTrue(sys.addMember("fleet1", "alice"), "Add first member");
    assertTrue(sys.addMember("fleet1", "bob"),   "Add second member");
    assertTrue(sys.getMemberCount("fleet1") == 2, "2 members registered");
    assertTrue(!sys.addMember("fleet1", "alice"), "Duplicate member rejected");
    assertTrue(!sys.addMember("fleet1", ""),      "Empty pilot ID rejected");
    assertTrue(sys.getMemberCount("fleet1") == 2, "Count unchanged after rejections");
}

static void testFAARRecordKillLoss() {
    std::cout << "\n=== FAAR: RecordKillLoss ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");
    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.addMember("fleet1", "bob");

    assertTrue(sys.recordKill("fleet1", "alice"), "Record kill for alice");
    assertTrue(sys.recordKill("fleet1", "alice"), "Record second kill for alice");
    assertTrue(sys.recordLoss("fleet1", "bob"),   "Record loss for bob");

    assertTrue(sys.getTotalKills("fleet1")  == 2, "2 total kills");
    assertTrue(sys.getTotalLosses("fleet1") == 1, "1 total loss");

    assertTrue(!sys.recordKill("fleet1", "charlie"),
               "Kill for unknown member rejected");
    assertTrue(!sys.recordLoss("fleet1", ""),
               "Loss for empty pilot rejected");
}

static void testFAARRecordDamage() {
    std::cout << "\n=== FAAR: RecordDamage ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");
    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.addMember("fleet1", "bob");

    assertTrue(sys.recordDamageDealt("fleet1",    "alice", 5000.0f),
               "Record damage dealt for alice");
    assertTrue(sys.recordDamageDealt("fleet1",    "bob",   3000.0f),
               "Record damage dealt for bob");
    assertTrue(sys.recordDamageReceived("fleet1", "alice",  800.0f),
               "Record damage received for alice");

    assertTrue(approxEqual(sys.getTotalDamage("fleet1"), 8000.0f),
               "Total damage dealt is 8000");
    assertTrue(!sys.recordDamageDealt("fleet1", "alice", -100.0f),
               "Negative damage dealt rejected");
    assertTrue(!sys.recordDamageReceived("fleet1", "alice", -50.0f),
               "Negative damage received rejected");
}

static void testFAARRecordLoot() {
    std::cout << "\n=== FAAR: RecordLoot ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");
    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.addMember("fleet1", "bob");

    assertTrue(sys.recordLootShared("fleet1", "alice", 1500.0f),
               "Record loot for alice");
    assertTrue(sys.recordLootShared("fleet1", "bob",    500.0f),
               "Record loot for bob");

    assertTrue(approxEqual(sys.getTotalLoot("fleet1"), 2000.0f),
               "Total loot is 2000");
    assertTrue(!sys.recordLootShared("fleet1", "alice", -100.0f),
               "Negative loot rejected");
}

static void testFAARFinalize() {
    std::cout << "\n=== FAAR: Finalize ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");

    // Cannot finalize from Idle
    assertTrue(!sys.finalizeReport("fleet1", 300.0f),
               "Finalize fails from Idle");

    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.recordKill("fleet1", "alice");

    assertTrue(sys.finalizeReport("fleet1", 300.0f), "Finalize succeeds from Recording");
    assertTrue(sys.getState("fleet1") == FAAR::State::Finalized,
               "State is Finalized after finalize");
    assertTrue(sys.getTotalReports("fleet1") == 1, "1 report counted");

    // Cannot finalize again
    assertTrue(!sys.finalizeReport("fleet1", 100.0f),
               "Cannot finalize a Finalized report");
    // Cannot record after finalization
    assertTrue(!sys.recordKill("fleet1", "alice"),
               "RecordKill blocked after finalization");
    assertTrue(!sys.addMember("fleet1", "bob"),
               "AddMember blocked after finalization");
}

static void testFAARRestartReport() {
    std::cout << "\n=== FAAR: RestartReport ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");

    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.recordKill("fleet1", "alice");
    sys.finalizeReport("fleet1", 200.0f);

    assertTrue(sys.getTotalReports("fleet1") == 1, "1 report before restart");

    // Start a new report — data resets, total_reports persists
    sys.startReport("fleet1");
    assertTrue(sys.getState("fleet1") == FAAR::State::Recording,
               "New recording state after restart");
    assertTrue(sys.getMemberCount("fleet1") == 0, "Members cleared on restart");
    assertTrue(sys.getTotalKills("fleet1")   == 0, "Kills cleared on restart");
    assertTrue(sys.getTotalReports("fleet1") == 1, "Total reports preserved across restart");
}

static void testFAARGetMVP() {
    std::cout << "\n=== FAAR: GetMVP ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");
    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.addMember("fleet1", "bob");
    sys.addMember("fleet1", "charlie");

    sys.recordDamageDealt("fleet1", "alice",   4000.0f);
    sys.recordDamageDealt("fleet1", "bob",     9500.0f);
    sys.recordDamageDealt("fleet1", "charlie", 2000.0f);

    assertTrue(sys.getMVP("fleet1") == "bob", "Bob is MVP with highest damage");

    assertTrue(sys.getMVP("nonexistent").empty(), "Empty MVP for missing entity");
}

static void testFAARFleetEfficiency() {
    std::cout << "\n=== FAAR: FleetEfficiency ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);
    world.createEntity("fleet1");
    sys.initialize("fleet1");
    sys.startReport("fleet1");
    sys.addMember("fleet1", "alice");
    sys.addMember("fleet1", "bob");

    // Zero kills and losses → efficiency 0
    assertTrue(approxEqual(sys.getFleetEfficiency("fleet1"), 0.0f),
               "Efficiency 0 with no kills or losses");

    sys.recordKill("fleet1", "alice");
    sys.recordKill("fleet1", "alice");
    sys.recordKill("fleet1", "alice");
    sys.recordLoss("fleet1", "bob");

    // 3 kills / (3+1) = 0.75
    assertTrue(approxEqual(sys.getFleetEfficiency("fleet1"), 0.75f),
               "Efficiency 75% with 3K/1L");
}

static void testFAARMissing() {
    std::cout << "\n=== FAAR: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetAfterActionReportSystem sys(&world);

    assertTrue(!sys.startReport("nx"),              "StartReport fails on missing");
    assertTrue(!sys.finalizeReport("nx", 0.0f),     "FinalizeReport fails on missing");
    assertTrue(!sys.addMember("nx", "p1"),          "AddMember fails on missing");
    assertTrue(!sys.recordKill("nx", "p1"),         "RecordKill fails on missing");
    assertTrue(!sys.recordLoss("nx", "p1"),         "RecordLoss fails on missing");
    assertTrue(!sys.recordDamageDealt("nx","p1",1.0f), "RecordDmgDealt fails on missing");
    assertTrue(!sys.recordDamageReceived("nx","p1",1.0f),"RecordDmgRcvd fails on missing");
    assertTrue(!sys.recordLootShared("nx","p1",1.0f), "RecordLoot fails on missing");
    assertTrue(sys.getState("nx") == FAAR::State::Idle, "Idle state on missing");
    assertTrue(sys.getMemberCount("nx") == 0, "0 members on missing");
    assertTrue(sys.getTotalKills("nx")   == 0, "0 kills on missing");
    assertTrue(sys.getTotalLosses("nx")  == 0, "0 losses on missing");
    assertTrue(approxEqual(sys.getTotalDamage("nx"), 0.0f), "0 damage on missing");
    assertTrue(approxEqual(sys.getTotalLoot("nx"),   0.0f), "0 loot on missing");
    assertTrue(sys.getMVP("nx").empty(),            "Empty MVP on missing");
    assertTrue(approxEqual(sys.getFleetEfficiency("nx"), 0.0f), "0 efficiency on missing");
}

void run_fleet_after_action_report_system_tests() {
    testFAARInit();
    testFAARStartReport();
    testFAARAddMember();
    testFAARRecordKillLoss();
    testFAARRecordDamage();
    testFAARRecordLoot();
    testFAARFinalize();
    testFAARRestartReport();
    testFAARGetMVP();
    testFAARFleetEfficiency();
    testFAARMissing();
}
