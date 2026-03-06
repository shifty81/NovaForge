// Tests for: Combat Log System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/combat_log_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Combat Log System Tests ====================

static void testCombatLogCreate() {
    std::cout << "\n=== CombatLog: Create ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    assertTrue(sys.initialize("cl1"), "Init succeeds");
    assertTrue(sys.getEntryCount("cl1") == 0, "No entries initially");
    assertTrue(sys.getEngagementCount("cl1") == 0, "No engagements initially");
    assertTrue(approxEqual(sys.getTotalDamageDealt("cl1"), 0.0f), "0 damage dealt");
    assertTrue(sys.getTotalKills("cl1") == 0, "0 kills");
    assertTrue(sys.getTotalLosses("cl1") == 0, "0 losses");
}

static void testCombatLogDamage() {
    std::cout << "\n=== CombatLog: LogDamage ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    assertTrue(sys.logDamage("cl1", "attacker1", "defender1", 0, 100.0f, "LaserTurret", true), "Log hit");
    assertTrue(sys.logDamage("cl1", "attacker1", "defender1", 2, 50.0f, "Railgun", false), "Log miss");
    assertTrue(sys.getEntryCount("cl1") == 2, "2 entries");
    assertTrue(approxEqual(sys.getTotalDamageDealt("cl1"), 100.0f), "100 damage dealt (miss not counted)");
}

static void testCombatLogStartEngagement() {
    std::cout << "\n=== CombatLog: StartEngagement ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    assertTrue(sys.startEngagement("cl1", "eng1"), "Start engagement succeeds");
    assertTrue(sys.getEngagementCount("cl1") == 1, "1 engagement");
    assertTrue(!sys.startEngagement("cl1", "eng1"), "Duplicate engagement rejected");
}

static void testCombatLogEndEngagement() {
    std::cout << "\n=== CombatLog: EndEngagement ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    sys.startEngagement("cl1", "eng1");
    assertTrue(sys.endEngagement("cl1", "eng1", 1), "End engagement Victory");
    assertTrue(!sys.endEngagement("cl1", "eng1", 2), "Double end rejected");
    assertTrue(!sys.endEngagement("cl1", "nonexistent", 1), "End missing engagement fails");
}

static void testCombatLogRecordKill() {
    std::cout << "\n=== CombatLog: RecordKill ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    sys.startEngagement("cl1", "eng1");
    assertTrue(sys.recordKill("cl1", "eng1"), "Record kill succeeds");
    assertTrue(sys.recordKill("cl1", "eng1"), "Record second kill succeeds");
    assertTrue(sys.getTotalKills("cl1") == 2, "2 total kills");
    assertTrue(!sys.recordKill("cl1", "nonexistent"), "Kill on missing engagement fails");
}

static void testCombatLogRecordLoss() {
    std::cout << "\n=== CombatLog: RecordLoss ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    sys.startEngagement("cl1", "eng1");
    assertTrue(sys.recordLoss("cl1", "eng1"), "Record loss succeeds");
    assertTrue(sys.getTotalLosses("cl1") == 1, "1 total loss");
}

static void testCombatLogAverageDPS() {
    std::cout << "\n=== CombatLog: AverageDPS ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    sys.startEngagement("cl1", "eng1");

    auto* entity = world.getEntity("cl1");
    auto* cl = entity->getComponent<components::CombatLog>();
    auto& eng = cl->engagements[0];
    eng.total_damage_dealt = 1000.0f;
    eng.duration = 10.0f;

    float dps = sys.getAverageDPS("cl1", "eng1");
    assertTrue(approxEqual(dps, 100.0f), "DPS is 100");
    assertTrue(approxEqual(sys.getAverageDPS("cl1", "nonexistent"), 0.0f), "0 DPS for missing");
}

static void testCombatLogEngagementDuration() {
    std::cout << "\n=== CombatLog: EngagementDuration ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");
    sys.startEngagement("cl1", "eng1");

    sys.update(5.0f);
    auto* entity = world.getEntity("cl1");
    auto* cl = entity->getComponent<components::CombatLog>();
    assertTrue(cl->engagements[0].duration > 4.9f, "Ongoing engagement duration tracked");

    sys.endEngagement("cl1", "eng1", 1);
    float dur = cl->engagements[0].duration;
    sys.update(5.0f);
    assertTrue(approxEqual(cl->engagements[0].duration, dur), "Ended engagement stops tracking");
}

static void testCombatLogMaxEntries() {
    std::cout << "\n=== CombatLog: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    world.createEntity("cl1");
    sys.initialize("cl1");

    auto* entity = world.getEntity("cl1");
    auto* cl = entity->getComponent<components::CombatLog>();
    cl->max_entries = 3;

    sys.logDamage("cl1", "a", "d", 0, 10.0f, "w1", true);
    sys.logDamage("cl1", "a", "d", 0, 20.0f, "w2", true);
    sys.logDamage("cl1", "a", "d", 0, 30.0f, "w3", true);
    sys.logDamage("cl1", "a", "d", 0, 40.0f, "w4", true);
    assertTrue(sys.getEntryCount("cl1") == 3, "Max entries enforced with eviction");
    assertTrue(approxEqual(cl->entries[0].damage_amount, 20.0f), "Oldest entry evicted");
}

static void testCombatLogMissing() {
    std::cout << "\n=== CombatLog: Missing ===" << std::endl;
    ecs::World world;
    systems::CombatLogSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.logDamage("nonexistent", "a", "d", 0, 10.0f, "w", true), "LogDamage fails");
    assertTrue(!sys.startEngagement("nonexistent", "e1"), "StartEngagement fails");
    assertTrue(!sys.endEngagement("nonexistent", "e1", 1), "EndEngagement fails");
    assertTrue(!sys.recordKill("nonexistent", "e1"), "RecordKill fails");
    assertTrue(!sys.recordLoss("nonexistent", "e1"), "RecordLoss fails");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(sys.getEngagementCount("nonexistent") == 0, "0 engagements on missing");
    assertTrue(approxEqual(sys.getTotalDamageDealt("nonexistent"), 0.0f), "0 damage on missing");
    assertTrue(approxEqual(sys.getTotalDamageReceived("nonexistent"), 0.0f), "0 received on missing");
    assertTrue(approxEqual(sys.getAverageDPS("nonexistent", "e1"), 0.0f), "0 DPS on missing");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(sys.getTotalLosses("nonexistent") == 0, "0 losses on missing");
}


void run_combat_log_system_tests() {
    testCombatLogCreate();
    testCombatLogDamage();
    testCombatLogStartEngagement();
    testCombatLogEndEngagement();
    testCombatLogRecordKill();
    testCombatLogRecordLoss();
    testCombatLogAverageDPS();
    testCombatLogEngagementDuration();
    testCombatLogMaxEntries();
    testCombatLogMissing();
}
