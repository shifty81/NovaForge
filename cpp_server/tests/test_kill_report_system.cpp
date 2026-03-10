// Tests for: KillReportSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/kill_report_system.h"

using namespace atlas;

// ==================== KillReportSystem Tests ====================

static void testKillReportInit() {
    std::cout << "\n=== KillReport: Init ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    assertTrue(sys.initialize("pilot1"), "Init succeeds");
    assertTrue(sys.getTotalKills("pilot1") == 0, "0 kills initially");
    assertTrue(sys.getTotalLosses("pilot1") == 0, "0 losses initially");
    assertTrue(sys.getKillEntryCount("pilot1") == 0, "0 kill entries");
    assertTrue(sys.getLossEntryCount("pilot1") == 0, "0 loss entries");
    assertTrue(approxEqual(sys.getTotalDamageDealt("pilot1"), 0.0f), "0 damage dealt");
    assertTrue(approxEqual(sys.getTotalDamageReceived("pilot1"), 0.0f), "0 damage received");
    assertTrue(sys.getPendingKillReports("pilot1") == 0, "0 pending kills");
    assertTrue(sys.getPendingLossReports("pilot1") == 0, "0 pending losses");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testKillReportRecordKill() {
    std::cout << "\n=== KillReport: RecordKill ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    assertTrue(sys.recordKill("pilot1", "pilot1", "enemy1", "Rifter", 5000.0f,
                               "Jita", "Jita IV-4"),
               "Record kill succeeds");
    assertTrue(sys.getTotalKills("pilot1") == 1, "1 total kill");
    assertTrue(sys.getKillEntryCount("pilot1") == 1, "1 kill entry");
    assertTrue(approxEqual(sys.getTotalDamageDealt("pilot1"), 5000.0f), "5000 damage dealt");
    assertTrue(sys.getPendingKillReports("pilot1") == 1, "1 pending kill report");

    auto entry = sys.getMostRecentKill("pilot1");
    assertTrue(entry.killer_id == "pilot1", "Killer ID stored");
    assertTrue(entry.victim_id == "enemy1", "Victim ID stored");
    assertTrue(entry.ship_type == "Rifter", "Ship type stored");
    assertTrue(approxEqual(entry.damage_dealt, 5000.0f), "Damage dealt stored");
    assertTrue(entry.system_id == "Jita", "System ID stored");
    assertTrue(entry.location == "Jita IV-4", "Location stored");
    assertTrue(!entry.acknowledged, "Entry not acknowledged");
}

static void testKillReportRecordLoss() {
    std::cout << "\n=== KillReport: RecordLoss ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    assertTrue(sys.recordLoss("pilot1", "enemy1", "pilot1", "Megathron", 80000.0f,
                               "Amarr", "Amarr VIII"),
               "Record loss succeeds");
    assertTrue(sys.getTotalLosses("pilot1") == 1, "1 total loss");
    assertTrue(sys.getLossEntryCount("pilot1") == 1, "1 loss entry");
    assertTrue(approxEqual(sys.getTotalDamageReceived("pilot1"), 80000.0f), "80000 damage received");
    assertTrue(sys.getPendingLossReports("pilot1") == 1, "1 pending loss report");

    auto entry = sys.getMostRecentLoss("pilot1");
    assertTrue(entry.killer_id == "enemy1", "Enemy killer ID stored");
    assertTrue(entry.victim_id == "pilot1", "Own victim ID stored");
    assertTrue(entry.ship_type == "Megathron", "Ship type stored");
}

static void testKillReportAcknowledge() {
    std::cout << "\n=== KillReport: Acknowledge ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    sys.recordKill("pilot1", "pilot1", "e1", "Rifter", 1000.0f, "Jita", "loc1");
    sys.recordKill("pilot1", "pilot1", "e2", "Drake", 2000.0f, "Jita", "loc2");
    sys.recordLoss("pilot1", "boss1", "pilot1", "Megathron", 5000.0f, "Null", "loc3");

    assertTrue(sys.getPendingKillReports("pilot1") == 2, "2 pending kills");
    assertTrue(sys.getPendingLossReports("pilot1") == 1, "1 pending loss");

    assertTrue(sys.acknowledgeKills("pilot1"), "Acknowledge kills succeeds");
    assertTrue(sys.getPendingKillReports("pilot1") == 0, "0 pending kills after ack");
    assertTrue(sys.getPendingLossReports("pilot1") == 1, "Loss still pending after kill ack");

    assertTrue(sys.acknowledgeLosses("pilot1"), "Acknowledge losses succeeds");
    assertTrue(sys.getPendingLossReports("pilot1") == 0, "0 pending losses after ack");

    // Verify entries marked as acknowledged
    auto* comp = world.getEntity("pilot1")->getComponent<components::KillReport>();
    for (const auto& k : comp->kills) {
        assertTrue(k.acknowledged, "Kill entry acknowledged");
    }
    assertTrue(comp->losses[0].acknowledged, "Loss entry acknowledged");
}

static void testKillReportMultipleKills() {
    std::cout << "\n=== KillReport: MultipleKills ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    sys.recordKill("pilot1", "pilot1", "e1", "Rifter", 1000.0f, "S1", "L1");
    sys.recordKill("pilot1", "pilot1", "e2", "Drake", 2000.0f, "S2", "L2");
    sys.recordKill("pilot1", "pilot1", "e3", "Raven", 3000.0f, "S3", "L3");

    assertTrue(sys.getTotalKills("pilot1") == 3, "3 total kills");
    assertTrue(approxEqual(sys.getTotalDamageDealt("pilot1"), 6000.0f), "6000 total damage");

    auto latest = sys.getMostRecentKill("pilot1");
    assertTrue(latest.victim_id == "e3", "Most recent kill is e3");
    assertTrue(latest.ship_type == "Raven", "Most recent ship type is Raven");
}

static void testKillReportMaxReports() {
    std::cout << "\n=== KillReport: MaxReports ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    auto* comp = world.getEntity("pilot1")->getComponent<components::KillReport>();
    comp->max_reports = 3;

    for (int i = 0; i < 5; ++i) {
        std::string victim = "enemy" + std::to_string(i);
        sys.recordKill("pilot1", "pilot1", victim, "Frigate", 1000.0f, "Jita", "loc");
    }

    assertTrue(sys.getKillEntryCount("pilot1") == 3, "Kill entries capped at max_reports");
    assertTrue(sys.getTotalKills("pilot1") == 5, "Total kills still 5 (counter not capped)");

    // Oldest entry should be evicted — most recent is enemy4
    auto latest = sys.getMostRecentKill("pilot1");
    assertTrue(latest.victim_id == "enemy4", "Most recent kill is enemy4");
    assertTrue(comp->kills[0].victim_id == "enemy2", "Oldest remaining is enemy2");
}

static void testKillReportTimestamps() {
    std::cout << "\n=== KillReport: Timestamps ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    sys.update(10.0f); // advance elapsed
    sys.recordKill("pilot1", "pilot1", "e1", "Rifter", 500.0f, "Jita", "loc");
    auto entry = sys.getMostRecentKill("pilot1");
    assertTrue(entry.timestamp >= 9.9f && entry.timestamp <= 10.1f,
               "Timestamp recorded at elapsed time ~10s");
}

static void testKillReportNoEntries() {
    std::cout << "\n=== KillReport: NoEntries ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);
    world.createEntity("pilot1");
    sys.initialize("pilot1");

    // getMostRecentKill/Loss on empty should return default
    auto kill = sys.getMostRecentKill("pilot1");
    assertTrue(kill.killer_id.empty(), "Empty kill entry on no kills");
    auto loss = sys.getMostRecentLoss("pilot1");
    assertTrue(loss.killer_id.empty(), "Empty loss entry on no losses");
}

static void testKillReportAcknowledgeFails() {
    std::cout << "\n=== KillReport: AcknowledgeFails ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);

    assertTrue(!sys.acknowledgeKills("nonexistent"), "AcknowledgeKills fails on missing");
    assertTrue(!sys.acknowledgeLosses("nonexistent"), "AcknowledgeLosses fails on missing");
}

static void testKillReportMissing() {
    std::cout << "\n=== KillReport: Missing ===" << std::endl;
    ecs::World world;
    systems::KillReportSystem sys(&world);

    assertTrue(!sys.recordKill("nonexistent", "a", "b", "S", 100.0f, "sys", "loc"),
               "RecordKill fails on missing");
    assertTrue(!sys.recordLoss("nonexistent", "a", "b", "S", 100.0f, "sys", "loc"),
               "RecordLoss fails on missing");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(sys.getTotalLosses("nonexistent") == 0, "0 losses on missing");
    assertTrue(sys.getKillEntryCount("nonexistent") == 0, "0 kill entries on missing");
    assertTrue(sys.getLossEntryCount("nonexistent") == 0, "0 loss entries on missing");
    assertTrue(approxEqual(sys.getTotalDamageDealt("nonexistent"), 0.0f), "0 damage on missing");
    assertTrue(approxEqual(sys.getTotalDamageReceived("nonexistent"), 0.0f), "0 recv on missing");
    assertTrue(sys.getPendingKillReports("nonexistent") == 0, "0 pending kills on missing");
    assertTrue(sys.getPendingLossReports("nonexistent") == 0, "0 pending losses on missing");
}

void run_kill_report_system_tests() {
    testKillReportInit();
    testKillReportRecordKill();
    testKillReportRecordLoss();
    testKillReportAcknowledge();
    testKillReportMultipleKills();
    testKillReportMaxReports();
    testKillReportTimestamps();
    testKillReportNoEntries();
    testKillReportAcknowledgeFails();
    testKillReportMissing();
}
