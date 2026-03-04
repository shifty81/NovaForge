// Tests for: PersistenceDelta System Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/persistence_delta_system.h"

using namespace atlas;

// ===== PersistenceDelta System Tests =====

static void testPersistenceDeltaCreate() {
    std::cout << "\n=== PersistenceDelta: Create ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeTracker("player1"), "Init tracker succeeds");
    assertTrue(sys.getActionCount("player1") == 0, "Initial action count is 0");
    assertTrue(sys.getEntryCount("player1") == 0, "Initial entry count is 0");
    assertTrue(approxEqual(sys.getTotalImpact("player1"), 0.0f), "Initial total impact is 0");
    assertTrue(!sys.isConsequenceTriggered("player1"), "No consequence initially");
}

static void testPersistenceDeltaRecord() {
    std::cout << "\n=== PersistenceDelta: Record ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    assertTrue(sys.recordAction("player1", "act1", "Combat", 5.0f, 0.1f, false), "Record action");
    assertTrue(sys.getActionCount("player1") == 1, "Action count is 1");
    assertTrue(sys.getEntryCount("player1") == 1, "Entry count is 1");
    assertTrue(approxEqual(sys.getTotalImpact("player1"), 5.0f), "Total impact is 5");
    assertTrue(approxEqual(sys.getPositiveImpact("player1"), 5.0f), "Positive impact is 5");
}

static void testPersistenceDeltaNegative() {
    std::cout << "\n=== PersistenceDelta: Negative ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    assertTrue(sys.recordAction("player1", "crime1", "Crime", -8.0f, 0.1f, false), "Record negative");
    assertTrue(approxEqual(sys.getNegativeImpact("player1"), -8.0f), "Negative impact is -8");
    assertTrue(approxEqual(sys.getTotalImpact("player1"), -8.0f), "Total impact is -8");
}

static void testPersistenceDeltaCategory() {
    std::cout << "\n=== PersistenceDelta: Category ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    sys.recordAction("player1", "a1", "Combat", 3.0f, 0.1f, false);
    sys.recordAction("player1", "a2", "Trade", 7.0f, 0.1f, false);
    sys.recordAction("player1", "a3", "Combat", 2.0f, 0.1f, false);
    assertTrue(approxEqual(sys.getCategoryImpact("player1", "Combat"), 5.0f), "Combat impact is 5");
    assertTrue(approxEqual(sys.getCategoryImpact("player1", "Trade"), 7.0f), "Trade impact is 7");
    assertTrue(approxEqual(sys.getCategoryImpact("player1", "Diplomacy"), 0.0f), "Diplomacy impact is 0");
}

static void testPersistenceDeltaDecay() {
    std::cout << "\n=== PersistenceDelta: Decay ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    sys.recordAction("player1", "a1", "Combat", 1.0f, 1.0f, false); // decays at 1.0/s
    sys.update(0.5f); // magnitude: 1.0 - 0.5 = 0.5
    assertTrue(approxEqual(sys.getTotalImpact("player1"), 0.5f), "Impact decayed to 0.5");
    sys.update(0.5f); // magnitude: 0.5 - 0.5 = 0.0 => removed
    assertTrue(sys.getEntryCount("player1") == 0, "Entry removed after full decay");
}

static void testPersistenceDeltaPermanent() {
    std::cout << "\n=== PersistenceDelta: Permanent ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    sys.recordAction("player1", "a1", "Diplomacy", 5.0f, 1.0f, true);
    sys.update(10.0f); // permanent entries don't decay
    assertTrue(approxEqual(sys.getTotalImpact("player1"), 5.0f), "Permanent entry unchanged");
    assertTrue(sys.getEntryCount("player1") == 1, "Permanent entry still exists");
}

static void testPersistenceDeltaConsequence() {
    std::cout << "\n=== PersistenceDelta: Consequence ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    sys.recordAction("player1", "a1", "Crime", -12.0f, 0.0f, true);
    sys.update(0.0f);
    assertTrue(sys.isConsequenceTriggered("player1"), "Consequence triggered at -12 (threshold 10)");
}

static void testPersistenceDeltaClear() {
    std::cout << "\n=== PersistenceDelta: Clear ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    sys.recordAction("player1", "a1", "Combat", 15.0f, 0.0f, true);
    sys.update(0.0f);
    assertTrue(sys.isConsequenceTriggered("player1"), "Consequence triggered");
    assertTrue(sys.clearConsequence("player1"), "Clear consequence succeeds");
    assertTrue(!sys.isConsequenceTriggered("player1"), "Consequence cleared");
}

static void testPersistenceDeltaMaxEntries() {
    std::cout << "\n=== PersistenceDelta: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    world.createEntity("player1");
    sys.initializeTracker("player1");
    // Record 105 actions (max is 100)
    for (int i = 0; i < 105; i++) {
        sys.recordAction("player1", "a" + std::to_string(i), "Combat", 1.0f, 0.0f, true);
    }
    assertTrue(sys.getEntryCount("player1") <= 100, "Entries capped at max_entries");
    assertTrue(sys.getActionCount("player1") == 105, "Action count tracks all recordings");
}

static void testPersistenceDeltaMissing() {
    std::cout << "\n=== PersistenceDelta: Missing ===" << std::endl;
    ecs::World world;
    systems::PersistenceDeltaSystem sys(&world);
    assertTrue(!sys.initializeTracker("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.recordAction("nonexistent", "a1", "Combat", 1.0f, 0.1f, false), "Record fails on missing");
    assertTrue(approxEqual(sys.getTotalImpact("nonexistent"), 0.0f), "0 impact on missing");
    assertTrue(approxEqual(sys.getCategoryImpact("nonexistent", "Combat"), 0.0f), "0 category on missing");
    assertTrue(sys.getActionCount("nonexistent") == 0, "0 actions on missing");
    assertTrue(!sys.isConsequenceTriggered("nonexistent"), "No consequence on missing");
    assertTrue(!sys.clearConsequence("nonexistent"), "Clear fails on missing");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(approxEqual(sys.getPositiveImpact("nonexistent"), 0.0f), "0 positive on missing");
    assertTrue(approxEqual(sys.getNegativeImpact("nonexistent"), 0.0f), "0 negative on missing");
}


void run_persistence_delta_system_tests() {
    testPersistenceDeltaCreate();
    testPersistenceDeltaRecord();
    testPersistenceDeltaNegative();
    testPersistenceDeltaCategory();
    testPersistenceDeltaDecay();
    testPersistenceDeltaPermanent();
    testPersistenceDeltaConsequence();
    testPersistenceDeltaClear();
    testPersistenceDeltaMaxEntries();
    testPersistenceDeltaMissing();
}
