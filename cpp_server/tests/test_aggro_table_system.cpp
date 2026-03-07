// Tests for: AggroTable System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/aggro_table_system.h"

using namespace atlas;

// ==================== AggroTable System Tests ====================

static void testAggroCreate() {
    std::cout << "\n=== AggroTable: Create ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    assertTrue(sys.initializeAggroTable("npc1", 2.0f, 5.0f), "Init aggro table succeeds");
    assertTrue(sys.getEntryCount("npc1") == 0, "No entries initially");
    assertTrue(sys.getTopThreat("npc1").empty(), "No top threat initially");
}

static void testAggroRecordThreat() {
    std::cout << "\n=== AggroTable: RecordThreat ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1");

    assertTrue(sys.recordThreat("npc1", "player1", 50.0f), "Record threat from player1");
    assertTrue(sys.getEntryCount("npc1") == 1, "1 entry");
    assertTrue(approxEqual(sys.getThreat("npc1", "player1"), 50.0f), "50 threat from player1");
    assertTrue(sys.getTopThreat("npc1") == "player1", "player1 is top threat");
}

static void testAggroAccumulate() {
    std::cout << "\n=== AggroTable: Accumulate ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1");

    sys.recordThreat("npc1", "player1", 30.0f);
    sys.recordThreat("npc1", "player1", 20.0f);
    assertTrue(approxEqual(sys.getThreat("npc1", "player1"), 50.0f), "Accumulated 30+20=50");
    assertTrue(sys.getTotalThreatEvents("npc1") == 2, "2 threat events");
    assertTrue(approxEqual(sys.getTotalThreatAccumulated("npc1"), 50.0f), "50 total accumulated");
}

static void testAggroTopThreat() {
    std::cout << "\n=== AggroTable: TopThreat ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1");

    sys.recordThreat("npc1", "player1", 30.0f);
    sys.recordThreat("npc1", "player2", 80.0f);
    sys.recordThreat("npc1", "player3", 50.0f);
    assertTrue(sys.getTopThreat("npc1") == "player2", "player2 has highest threat");
    assertTrue(sys.getEntryCount("npc1") == 3, "3 entries");
}

static void testAggroDecay() {
    std::cout << "\n=== AggroTable: Decay ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1", 10.0f, 1.0f);  // 10/sec decay, 1s delay

    sys.recordThreat("npc1", "player1", 50.0f);

    // Advance 1.5 seconds (past decay delay) - should decay 5 (0.5s * 10/sec)
    for (int i = 0; i < 15; ++i) sys.update(0.1f);

    float threat = sys.getThreat("npc1", "player1");
    assertTrue(threat < 50.0f, "Threat decayed below 50");
    assertTrue(threat > 40.0f, "Threat still above 40");
}

static void testAggroDecayRemoval() {
    std::cout << "\n=== AggroTable: DecayRemoval ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1", 100.0f, 0.5f);  // fast decay, short delay

    sys.recordThreat("npc1", "player1", 10.0f);

    // Advance enough for full decay
    for (int i = 0; i < 20; ++i) sys.update(0.1f);

    assertTrue(sys.getEntryCount("npc1") == 0, "Entry removed after full decay");
    assertTrue(sys.getTopThreat("npc1").empty(), "No top threat after decay");
}

static void testAggroMaxEntries() {
    std::cout << "\n=== AggroTable: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1");
    auto* entity = world.getEntity("npc1");
    auto* at = entity->getComponent<components::AggroTable>();
    at->max_entries = 3;

    sys.recordThreat("npc1", "p1", 10.0f);
    sys.recordThreat("npc1", "p2", 20.0f);
    sys.recordThreat("npc1", "p3", 30.0f);
    assertTrue(!sys.recordThreat("npc1", "p4", 40.0f), "4th entry rejected at max=3");
    assertTrue(sys.getEntryCount("npc1") == 3, "Still 3 entries");
}

static void testAggroClear() {
    std::cout << "\n=== AggroTable: Clear ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1");
    sys.recordThreat("npc1", "p1", 10.0f);
    sys.recordThreat("npc1", "p2", 20.0f);

    assertTrue(sys.clearTable("npc1"), "Clear succeeds");
    assertTrue(sys.getEntryCount("npc1") == 0, "0 entries after clear");
}

static void testAggroDuplicateInit() {
    std::cout << "\n=== AggroTable: DuplicateInit ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    assertTrue(sys.initializeAggroTable("npc1"), "First init succeeds");
    assertTrue(!sys.initializeAggroTable("npc1"), "Duplicate init rejected");
}

static void testAggroMissing() {
    std::cout << "\n=== AggroTable: Missing ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    assertTrue(!sys.initializeAggroTable("nonexistent"), "Init fails on missing");
    assertTrue(!sys.recordThreat("nonexistent", "p1", 10.0f), "Record fails on missing");
    assertTrue(approxEqual(sys.getThreat("nonexistent", "p1"), 0.0f), "0 threat on missing");
    assertTrue(sys.getTopThreat("nonexistent").empty(), "No top threat on missing");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(!sys.clearTable("nonexistent"), "Clear fails on missing");
}

static void testAggroInvalidInput() {
    std::cout << "\n=== AggroTable: InvalidInput ===" << std::endl;
    ecs::World world;
    systems::AggroTableSystem sys(&world);
    world.createEntity("npc1");
    sys.initializeAggroTable("npc1");

    assertTrue(!sys.recordThreat("npc1", "", 10.0f), "Empty attacker rejected");
    assertTrue(!sys.recordThreat("npc1", "p1", 0.0f), "Zero threat rejected");
    assertTrue(!sys.recordThreat("npc1", "p1", -5.0f), "Negative threat rejected");
}

void run_aggro_table_system_tests() {
    testAggroCreate();
    testAggroRecordThreat();
    testAggroAccumulate();
    testAggroTopThreat();
    testAggroDecay();
    testAggroDecayRemoval();
    testAggroMaxEntries();
    testAggroClear();
    testAggroDuplicateInit();
    testAggroMissing();
    testAggroInvalidInput();
}
