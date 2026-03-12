// Tests for: MiningLedgerSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/mining_ledger_system.h"

using namespace atlas;

// ==================== MiningLedgerSystem Tests ====================

static void testMiningLedgerInit() {
    std::cout << "\n=== MiningLedger: Init ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "pilot_1"), "Init succeeds");
    assertTrue(sys.getEntryCount("p1") == 0, "Zero entries initially");
    assertTrue(sys.getTotalEntries("p1") == 0, "Zero total entries initially");
    assertTrue(sys.getTotalQuantity("p1") == 0, "Zero quantity initially");
    assertTrue(approxEqual(sys.getTotalIsk("p1"), 0.0f), "Zero ISK initially");
    assertTrue(sys.getOwner("p1") == "pilot_1", "Owner set correctly");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testMiningLedgerAddEntry() {
    std::cout << "\n=== MiningLedger: AddEntry ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.addEntry("p1", "e1", "Veldspar", 100, 5000.0f), "Add Veldspar entry");
    assertTrue(sys.addEntry("p1", "e2", "Kernite", 50, 15000.0f), "Add Kernite entry");
    assertTrue(sys.getEntryCount("p1") == 2, "Two entries stored");
    assertTrue(sys.getTotalEntries("p1") == 2, "Total entries is 2");
    assertTrue(sys.getTotalQuantity("p1") == 150, "Total quantity is 150");
    assertTrue(approxEqual(sys.getTotalIsk("p1"), 20000.0f), "Total ISK is 20000");
    assertTrue(sys.hasEntry("p1", "e1"), "Has entry e1");
    assertTrue(sys.hasEntry("p1", "e2"), "Has entry e2");
    assertTrue(!sys.hasEntry("p1", "e3"), "No entry e3");
}

static void testMiningLedgerAddValidation() {
    std::cout << "\n=== MiningLedger: AddValidation ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(!sys.addEntry("p1", "", "Veldspar", 100, 5000.0f), "Empty id rejected");
    assertTrue(!sys.addEntry("p1", "e1", "", 100, 5000.0f), "Empty ore_type rejected");
    assertTrue(!sys.addEntry("p1", "e1", "Veldspar", 0, 5000.0f), "Zero quantity rejected");
    assertTrue(!sys.addEntry("p1", "e1", "Veldspar", -1, 5000.0f), "Negative quantity rejected");
    assertTrue(!sys.addEntry("p1", "e1", "Veldspar", 100, -5.0f), "Negative ISK rejected");
    assertTrue(sys.addEntry("p1", "e1", "Veldspar", 100, 0.0f), "Zero ISK allowed");
    assertTrue(!sys.addEntry("p1", "e1", "Veldspar", 50, 100.0f), "Duplicate entry_id rejected");
    assertTrue(sys.getEntryCount("p1") == 1, "Only 1 valid entry");
}

static void testMiningLedgerPurgeOldest() {
    std::cout << "\n=== MiningLedger: PurgeOldest ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.setMaxEntries("p1", 3);

    sys.addEntry("p1", "e1", "Veldspar", 10, 100.0f);
    sys.addEntry("p1", "e2", "Kernite", 20, 200.0f);
    sys.addEntry("p1", "e3", "Mercoxit", 30, 300.0f);
    assertTrue(sys.getEntryCount("p1") == 3, "3 entries at cap");

    // Adding 4th should purge oldest
    assertTrue(sys.addEntry("p1", "e4", "Arkonor", 40, 400.0f), "4th entry added");
    assertTrue(sys.getEntryCount("p1") == 3, "Still 3 entries after purge");
    assertTrue(!sys.hasEntry("p1", "e1"), "Oldest (e1) purged");
    assertTrue(sys.hasEntry("p1", "e4"), "Newest (e4) present");
    assertTrue(sys.getTotalEntries("p1") == 4, "Total entries is 4 (lifetime)");
    assertTrue(sys.getTotalQuantity("p1") == 100, "Total quantity is 100 (lifetime)");
}

static void testMiningLedgerRemoveEntry() {
    std::cout << "\n=== MiningLedger: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.addEntry("p1", "e1", "Veldspar", 100, 5000.0f);
    sys.addEntry("p1", "e2", "Kernite", 50, 3000.0f);

    assertTrue(sys.removeEntry("p1", "e1"), "Remove e1 succeeds");
    assertTrue(sys.getEntryCount("p1") == 1, "1 entry remaining");
    assertTrue(!sys.hasEntry("p1", "e1"), "e1 gone");
    assertTrue(sys.hasEntry("p1", "e2"), "e2 still present");
    assertTrue(!sys.removeEntry("p1", "e1"), "Remove nonexistent fails");
    assertTrue(!sys.removeEntry("p1", "unknown"), "Remove unknown fails");
}

static void testMiningLedgerClear() {
    std::cout << "\n=== MiningLedger: Clear ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.addEntry("p1", "e1", "Veldspar", 100, 5000.0f);
    sys.addEntry("p1", "e2", "Kernite", 50, 3000.0f);

    assertTrue(sys.clearLedger("p1"), "Clear succeeds");
    assertTrue(sys.getEntryCount("p1") == 0, "0 entries after clear");
    // Lifetime aggregates persist
    assertTrue(sys.getTotalEntries("p1") == 2, "Total entries lifetime still 2");
    assertTrue(sys.getTotalQuantity("p1") == 150, "Total quantity lifetime 150");
}

static void testMiningLedgerOreTypeQueries() {
    std::cout << "\n=== MiningLedger: OreTypeQueries ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    sys.addEntry("p1", "e1", "Veldspar", 100, 5000.0f);
    sys.addEntry("p1", "e2", "Veldspar", 200, 10000.0f);
    sys.addEntry("p1", "e3", "Kernite", 50, 8000.0f);

    assertTrue(sys.getQuantityByOreType("p1", "Veldspar") == 300,
               "300 Veldspar quantity");
    assertTrue(approxEqual(sys.getIskByOreType("p1", "Veldspar"), 15000.0f),
               "15000 Veldspar ISK");
    assertTrue(sys.getQuantityByOreType("p1", "Kernite") == 50,
               "50 Kernite quantity");
    assertTrue(approxEqual(sys.getIskByOreType("p1", "Kernite"), 8000.0f),
               "8000 Kernite ISK");
    assertTrue(sys.getQuantityByOreType("p1", "Unknown") == 0,
               "0 for unknown ore type");
    assertTrue(approxEqual(sys.getIskByOreType("p1", "Unknown"), 0.0f),
               "0 ISK for unknown ore type");
}

static void testMiningLedgerSetOwner() {
    std::cout << "\n=== MiningLedger: SetOwner ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.setOwner("p1", "corp_123"), "Set owner succeeds");
    assertTrue(sys.getOwner("p1") == "corp_123", "Owner updated");
    assertTrue(!sys.setOwner("p1", ""), "Empty owner rejected");
    assertTrue(!sys.setOwner("nonexistent", "owner"), "Fails on missing entity");
}

static void testMiningLedgerSetMaxEntries() {
    std::cout << "\n=== MiningLedger: SetMaxEntries ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.setMaxEntries("p1", 10), "Set max entries to 10");
    assertTrue(!sys.setMaxEntries("p1", 0), "Zero rejected");
    assertTrue(!sys.setMaxEntries("p1", -1), "Negative rejected");
    assertTrue(!sys.setMaxEntries("nonexistent", 10), "Fails on missing entity");
}

static void testMiningLedgerMissing() {
    std::cout << "\n=== MiningLedger: Missing ===" << std::endl;
    ecs::World world;
    systems::MiningLedgerSystem sys(&world);

    assertTrue(!sys.addEntry("none", "e1", "Veldspar", 100, 5000.0f),
               "Add fails on missing");
    assertTrue(!sys.removeEntry("none", "e1"), "Remove fails on missing");
    assertTrue(!sys.clearLedger("none"), "Clear fails on missing");
    assertTrue(!sys.setOwner("none", "owner"), "SetOwner fails on missing");
    assertTrue(!sys.setMaxEntries("none", 10), "SetMax fails on missing");
    assertTrue(sys.getEntryCount("none") == 0, "0 count on missing");
    assertTrue(sys.getTotalEntries("none") == 0, "0 total on missing");
    assertTrue(sys.getTotalQuantity("none") == 0, "0 quantity on missing");
    assertTrue(approxEqual(sys.getTotalIsk("none"), 0.0f), "0 ISK on missing");
    assertTrue(sys.getOwner("none") == "", "Empty owner on missing");
    assertTrue(!sys.hasEntry("none", "e1"), "No entry on missing");
    assertTrue(sys.getQuantityByOreType("none", "Veldspar") == 0,
               "0 ore quantity on missing");
    assertTrue(approxEqual(sys.getIskByOreType("none", "Veldspar"), 0.0f),
               "0 ore ISK on missing");
}

void run_mining_ledger_system_tests() {
    testMiningLedgerInit();
    testMiningLedgerAddEntry();
    testMiningLedgerAddValidation();
    testMiningLedgerPurgeOldest();
    testMiningLedgerRemoveEntry();
    testMiningLedgerClear();
    testMiningLedgerOreTypeQueries();
    testMiningLedgerSetOwner();
    testMiningLedgerSetMaxEntries();
    testMiningLedgerMissing();
}
