// Tests for: CorpTaxLedgerSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/corp_tax_ledger_system.h"

using namespace atlas;

// ==================== CorpTaxLedgerSystem Tests ====================

static void testCorpTaxLedgerInit() {
    std::cout << "\n=== CorpTaxLedger: Init ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    assertTrue(sys.initialize("c1"), "Init succeeds");
    assertTrue(sys.getEntryCount("c1") == 0, "Zero entries initially");
    assertTrue(sys.getTotalEntriesEver("c1") == 0, "Zero total entries");
    assertTrue(sys.getTotalCollected("c1") == 0.0, "Zero collected");
    assertTrue(sys.getCorpId("c1") == "", "Empty corp_id initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCorpTaxLedgerSetConfig() {
    std::cout << "\n=== CorpTaxLedger: SetConfig ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    assertTrue(sys.setCorpId("c1", "CORP001"), "Set corp_id");
    assertTrue(sys.getCorpId("c1") == "CORP001", "Corp_id is CORP001");
    assertTrue(!sys.setCorpId("c1", ""), "Empty corp_id rejected");
    assertTrue(sys.setDefaultTaxRate("c1", 0.15), "Set rate 15%");
    assertTrue(approxEqual(sys.getDefaultTaxRate("c1"), 0.15), "Rate is 0.15");
    assertTrue(!sys.setDefaultTaxRate("c1", -0.1), "Negative rate rejected");
    assertTrue(!sys.setDefaultTaxRate("c1", 1.1), "Rate > 1 rejected");
    assertTrue(sys.setDefaultTaxRate("c1", 0.0), "Zero rate allowed");
    assertTrue(sys.setDefaultTaxRate("c1", 1.0), "Rate = 1.0 allowed");
    assertTrue(sys.setMaxEntries("c1", 50), "Set max entries 50");
    assertTrue(!sys.setMaxEntries("c1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxEntries("c1", -5), "Negative max rejected");
}

static void testCorpTaxLedgerAddEntry() {
    std::cout << "\n=== CorpTaxLedger: AddEntry ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    assertTrue(sys.addEntry("c1", "e1", TT::Bounty, "pilot1", 1000.0),
               "Add bounty entry");
    assertTrue(sys.getEntryCount("c1") == 1, "One entry");
    assertTrue(sys.hasEntry("c1", "e1"), "Has e1");
    assertTrue(sys.getTotalEntriesEver("c1") == 1, "1 total ever");
    // Default tax rate is 10%, so collected = 1000 * 0.10 = 100
    assertTrue(approxEqual(sys.getTotalCollected("c1"), 100.0), "Collected 100.0");

    assertTrue(sys.addEntry("c1", "e2", TT::Mission, "pilot2", 5000.0),
               "Add mission entry");
    assertTrue(sys.getEntryCount("c1") == 2, "Two entries");
    assertTrue(approxEqual(sys.getTotalCollected("c1"), 600.0), "Collected 600.0 total");
}

static void testCorpTaxLedgerAddEntryWithRate() {
    std::cout << "\n=== CorpTaxLedger: AddEntryWithRate ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    assertTrue(sys.addEntryWithRate("c1", "e1", TT::PI, "pilot1", 2000.0, 0.05),
               "Add PI entry at 5%");
    assertTrue(approxEqual(sys.getTotalCollected("c1"), 100.0), "Collected 100 from PI");
    assertTrue(sys.addEntryWithRate("c1", "e2", TT::Industry, "pilot1", 10000.0, 0.25),
               "Add industry at 25%");
    assertTrue(approxEqual(sys.getTotalCollected("c1"), 2600.0), "Total 2600");
}

static void testCorpTaxLedgerAddValidation() {
    std::cout << "\n=== CorpTaxLedger: AddValidation ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    assertTrue(!sys.addEntry("c1", "", TT::Bounty, "p1", 100.0), "Empty id rejected");
    assertTrue(!sys.addEntry("c1", "e1", TT::Bounty, "", 100.0), "Empty member rejected");
    assertTrue(!sys.addEntry("c1", "e1", TT::Bounty, "p1", 0.0), "Zero amount rejected");
    assertTrue(!sys.addEntry("c1", "e1", TT::Bounty, "p1", -100.0), "Negative amount rejected");
    assertTrue(sys.addEntry("c1", "e1", TT::Bounty, "p1", 100.0), "Valid entry added");
    assertTrue(!sys.addEntry("c1", "e1", TT::Bounty, "p2", 200.0), "Duplicate id rejected");
    assertTrue(!sys.addEntryWithRate("c1", "e2", TT::Bounty, "p1", 100.0, -0.1),
               "Negative rate rejected");
    assertTrue(!sys.addEntryWithRate("c1", "e3", TT::Bounty, "p1", 100.0, 1.1),
               "Rate > 1 rejected");
}

static void testCorpTaxLedgerCapPurge() {
    std::cout << "\n=== CorpTaxLedger: CapPurge ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");
    sys.setMaxEntries("c1", 5);

    using TT = components::CorpTaxLedgerState::TaxType;
    for (int i = 0; i < 5; i++) {
        std::string id = "e" + std::to_string(i);
        assertTrue(sys.addEntry("c1", id, TT::Bounty, "p1", 100.0),
                   "Add entry within cap");
    }
    assertTrue(sys.getEntryCount("c1") == 5, "5 entries at cap");
    assertTrue(sys.hasEntry("c1", "e0"), "e0 present");

    // Adding e5 purges e0
    assertTrue(sys.addEntry("c1", "e5", TT::Bounty, "p1", 100.0),
               "Add entry beyond cap, oldest purged");
    assertTrue(sys.getEntryCount("c1") == 5, "Still 5 entries");
    assertTrue(!sys.hasEntry("c1", "e0"), "e0 purged");
    assertTrue(sys.hasEntry("c1", "e5"), "e5 present");
    assertTrue(sys.getTotalEntriesEver("c1") == 6, "6 total ever");
}

static void testCorpTaxLedgerRemove() {
    std::cout << "\n=== CorpTaxLedger: Remove ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    sys.addEntry("c1", "e1", TT::Bounty, "p1", 1000.0);
    sys.addEntry("c1", "e2", TT::Mission, "p2", 2000.0);

    assertTrue(sys.removeEntry("c1", "e1"), "Remove e1");
    assertTrue(sys.getEntryCount("c1") == 1, "1 entry left");
    assertTrue(!sys.hasEntry("c1", "e1"), "e1 removed");
    assertTrue(sys.hasEntry("c1", "e2"), "e2 present");
    assertTrue(!sys.removeEntry("c1", "e1"), "Remove nonexistent fails");
    assertTrue(!sys.removeEntry("c1", "unknown"), "Remove unknown fails");
}

static void testCorpTaxLedgerClear() {
    std::cout << "\n=== CorpTaxLedger: Clear ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    sys.addEntry("c1", "e1", TT::Bounty, "p1", 1000.0);
    sys.addEntry("c1", "e2", TT::Mission, "p2", 2000.0);

    assertTrue(sys.clearLedger("c1"), "Clear succeeds");
    assertTrue(sys.getEntryCount("c1") == 0, "0 entries after clear");
    assertTrue(!sys.hasEntry("c1", "e1"), "e1 gone");
    assertTrue(!sys.hasEntry("c1", "e2"), "e2 gone");
    // total_collected and total_entries_ever preserved
    assertTrue(sys.getTotalEntriesEver("c1") == 2, "Total ever preserved");
    assertTrue(sys.getTotalCollected("c1") > 0, "Total collected preserved");
}

static void testCorpTaxLedgerQueryByType() {
    std::cout << "\n=== CorpTaxLedger: QueryByType ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    sys.addEntryWithRate("c1", "e1", TT::Bounty, "p1", 1000.0, 0.10);
    sys.addEntryWithRate("c1", "e2", TT::Bounty, "p1", 2000.0, 0.10);
    sys.addEntryWithRate("c1", "e3", TT::Mission, "p2", 5000.0, 0.10);
    sys.addEntryWithRate("c1", "e4", TT::PI, "p3", 3000.0, 0.05);
    sys.addEntryWithRate("c1", "e5", TT::Market, "p1", 10000.0, 0.02);

    assertTrue(sys.getCountByType("c1", TT::Bounty) == 2, "2 bounty entries");
    assertTrue(sys.getCountByType("c1", TT::Mission) == 1, "1 mission entry");
    assertTrue(sys.getCountByType("c1", TT::PI) == 1, "1 PI entry");
    assertTrue(sys.getCountByType("c1", TT::Market) == 1, "1 market entry");
    assertTrue(sys.getCountByType("c1", TT::Industry) == 0, "0 industry entries");
    assertTrue(approxEqual(sys.getCollectedByType("c1", TT::Bounty), 300.0), "300 bounty tax");
    assertTrue(approxEqual(sys.getCollectedByType("c1", TT::Mission), 500.0), "500 mission tax");
    assertTrue(approxEqual(sys.getCollectedByType("c1", TT::PI), 150.0), "150 PI tax");
    assertTrue(approxEqual(sys.getCollectedByType("c1", TT::Market), 200.0), "200 market tax");
}

static void testCorpTaxLedgerQueryByMember() {
    std::cout << "\n=== CorpTaxLedger: QueryByMember ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);
    world.createEntity("c1");
    sys.initialize("c1");

    using TT = components::CorpTaxLedgerState::TaxType;
    sys.addEntryWithRate("c1", "e1", TT::Bounty, "pilot_a", 1000.0, 0.10);
    sys.addEntryWithRate("c1", "e2", TT::Bounty, "pilot_b", 2000.0, 0.10);
    sys.addEntryWithRate("c1", "e3", TT::Mission, "pilot_a", 5000.0, 0.10);

    assertTrue(approxEqual(sys.getCollectedByMember("c1", "pilot_a"), 600.0),
               "600 from pilot_a");
    assertTrue(approxEqual(sys.getCollectedByMember("c1", "pilot_b"), 200.0),
               "200 from pilot_b");
    assertTrue(approxEqual(sys.getCollectedByMember("c1", "unknown"), 0.0),
               "0 from unknown");
}

static void testCorpTaxLedgerMissing() {
    std::cout << "\n=== CorpTaxLedger: Missing ===" << std::endl;
    ecs::World world;
    systems::CorpTaxLedgerSystem sys(&world);

    using TT = components::CorpTaxLedgerState::TaxType;
    assertTrue(!sys.addEntry("none", "e1", TT::Bounty, "p1", 1000.0),
               "Add fails on missing");
    assertTrue(!sys.addEntryWithRate("none", "e1", TT::Bounty, "p1", 1000.0, 0.1),
               "AddWithRate fails on missing");
    assertTrue(!sys.removeEntry("none", "e1"), "Remove fails on missing");
    assertTrue(!sys.clearLedger("none"), "Clear fails on missing");
    assertTrue(!sys.setCorpId("none", "corp"), "SetCorpId fails on missing");
    assertTrue(!sys.setDefaultTaxRate("none", 0.1), "SetRate fails on missing");
    assertTrue(!sys.setMaxEntries("none", 10), "SetMax fails on missing");
    assertTrue(sys.getEntryCount("none") == 0, "0 count on missing");
    assertTrue(sys.getTotalEntriesEver("none") == 0, "0 total on missing");
    assertTrue(sys.getTotalCollected("none") == 0.0, "0 collected on missing");
    assertTrue(sys.getDefaultTaxRate("none") == 0.0, "0 rate on missing");
    assertTrue(sys.getCorpId("none") == "", "Empty corp on missing");
    assertTrue(!sys.hasEntry("none", "e1"), "No entry on missing");
    assertTrue(sys.getCollectedByType("none", TT::Bounty) == 0.0,
               "0 type on missing");
    assertTrue(sys.getCollectedByMember("none", "p1") == 0.0,
               "0 member on missing");
    assertTrue(sys.getCountByType("none", TT::Bounty) == 0,
               "0 type count on missing");
}

void run_corp_tax_ledger_system_tests() {
    testCorpTaxLedgerInit();
    testCorpTaxLedgerSetConfig();
    testCorpTaxLedgerAddEntry();
    testCorpTaxLedgerAddEntryWithRate();
    testCorpTaxLedgerAddValidation();
    testCorpTaxLedgerCapPurge();
    testCorpTaxLedgerRemove();
    testCorpTaxLedgerClear();
    testCorpTaxLedgerQueryByType();
    testCorpTaxLedgerQueryByMember();
    testCorpTaxLedgerMissing();
}
