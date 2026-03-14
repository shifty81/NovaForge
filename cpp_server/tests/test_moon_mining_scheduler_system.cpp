// Tests for: MoonMiningSchedulerSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/moon_mining_scheduler_system.h"

using namespace atlas;

// ==================== MoonMiningSchedulerSystem Tests ====================

static void testMoonMiningInit() {
    std::cout << "\n=== MoonMining: Init ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    assertTrue(sys.initialize("r1"), "Init succeeds");
    assertTrue(sys.getExtractionCount("r1") == 0, "Zero extractions");
    assertTrue(sys.getActiveExtractionCount("r1") == 0, "Zero active");
    assertTrue(sys.getTotalExtractionsStarted("r1") == 0, "Zero started");
    assertTrue(sys.getTotalFractured("r1") == 0, "Zero fractured");
    assertTrue(approxEqual(sys.getTotalYield("r1"), 0.0f), "Zero yield");
    assertTrue(sys.getStructureId("r1") == "", "Empty structure id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testMoonMiningConfig() {
    std::cout << "\n=== MoonMining: Config ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    assertTrue(sys.setStructureId("r1", "refinery_001"), "Set structure id");
    assertTrue(sys.getStructureId("r1") == "refinery_001", "Structure id set");
    assertTrue(!sys.setStructureId("r1", ""), "Empty structure rejected");
    assertTrue(sys.setMaxExtractions("r1", 3), "Set max extractions");
    assertTrue(!sys.setMaxExtractions("r1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxExtractions("r1", -1), "Negative max rejected");
}

static void testMoonMiningStartExtraction() {
    std::cout << "\n=== MoonMining: StartExtraction ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    assertTrue(sys.startExtraction("r1", "ext1", "moon_a", "Veldspar", 3600.0f, 50000.0f),
               "Start extraction");
    assertTrue(sys.getExtractionCount("r1") == 1, "1 extraction");
    assertTrue(sys.hasExtraction("r1", "ext1"), "Has ext1");
    assertTrue(sys.getActiveExtractionCount("r1") == 1, "1 active");
    assertTrue(sys.getTotalExtractionsStarted("r1") == 1, "1 started");
    assertTrue(sys.getExtractionStatus("r1", "ext1") == "Extracting", "Status Extracting");
    assertTrue(approxEqual(sys.getTimeRemaining("r1", "ext1"), 3600.0f), "3600 remaining");
}

static void testMoonMiningStartValidation() {
    std::cout << "\n=== MoonMining: StartValidation ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    assertTrue(!sys.startExtraction("r1", "", "moon", "ore", 100.0f, 100.0f),
               "Empty id rejected");
    assertTrue(!sys.startExtraction("r1", "ext1", "", "ore", 100.0f, 100.0f),
               "Empty moon rejected");
    assertTrue(!sys.startExtraction("r1", "ext1", "moon", "", 100.0f, 100.0f),
               "Empty ore rejected");
    assertTrue(!sys.startExtraction("r1", "ext1", "moon", "ore", 0.0f, 100.0f),
               "Zero duration rejected");
    assertTrue(!sys.startExtraction("r1", "ext1", "moon", "ore", -10.0f, 100.0f),
               "Negative duration rejected");
    assertTrue(!sys.startExtraction("r1", "ext1", "moon", "ore", 100.0f, 0.0f),
               "Zero yield rejected");
    assertTrue(!sys.startExtraction("r1", "ext1", "moon", "ore", 100.0f, -50.0f),
               "Negative yield rejected");
    assertTrue(sys.startExtraction("r1", "ext1", "moon", "ore", 100.0f, 100.0f),
               "Valid start");
    assertTrue(!sys.startExtraction("r1", "ext1", "moon", "ore", 100.0f, 100.0f),
               "Duplicate id rejected");
}

static void testMoonMiningMaxCap() {
    std::cout << "\n=== MoonMining: MaxCap ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");
    sys.setMaxExtractions("r1", 3);

    for (int i = 0; i < 3; i++) {
        std::string id = "ext" + std::to_string(i);
        assertTrue(sys.startExtraction("r1", id, "moon", "ore", 100.0f, 100.0f),
                   "Extraction within cap");
    }
    assertTrue(!sys.startExtraction("r1", "ext3", "moon", "ore", 100.0f, 100.0f),
               "Blocked at max");
    assertTrue(sys.getExtractionCount("r1") == 3, "3 extractions");
}

static void testMoonMiningTickCountdown() {
    std::cout << "\n=== MoonMining: TickCountdown ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    sys.startExtraction("r1", "ext1", "moon_a", "Spodumain", 100.0f, 5000.0f);

    // Simulate tick
    sys.update(50.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("r1", "ext1"), 50.0f), "50 remaining");
    assertTrue(sys.getExtractionStatus("r1", "ext1") == "Extracting", "Still extracting");

    sys.update(50.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("r1", "ext1"), 0.0f), "0 remaining");
    assertTrue(sys.getExtractionStatus("r1", "ext1") == "Ready", "Status Ready");
    assertTrue(sys.getActiveExtractionCount("r1") == 0, "0 active after ready");
}

static void testMoonMiningFracture() {
    std::cout << "\n=== MoonMining: Fracture ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    sys.startExtraction("r1", "ext1", "moon_a", "Mercoxit", 10.0f, 8000.0f);
    sys.update(10.0f); // becomes Ready

    assertTrue(sys.fractureExtraction("r1", "ext1"), "Fracture succeeds");
    assertTrue(sys.getExtractionStatus("r1", "ext1") == "Fractured", "Status Fractured");
    assertTrue(sys.getTotalFractured("r1") == 1, "1 total fractured");
    assertTrue(approxEqual(sys.getTotalYield("r1"), 8000.0f), "8000 yield accumulated");

    // Cannot fracture again
    assertTrue(!sys.fractureExtraction("r1", "ext1"), "Double fracture rejected");
}

static void testMoonMiningCancel() {
    std::cout << "\n=== MoonMining: Cancel ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    sys.startExtraction("r1", "ext1", "moon_a", "Arkonor", 1000.0f, 2000.0f);

    assertTrue(sys.cancelExtraction("r1", "ext1"), "Cancel extraction");
    assertTrue(sys.getExtractionStatus("r1", "ext1") == "Idle", "Status Idle after cancel");
    assertTrue(sys.getActiveExtractionCount("r1") == 0, "0 active after cancel");
    assertTrue(!sys.cancelExtraction("r1", "ext1"), "Cannot cancel non-extracting");
    assertTrue(!sys.cancelExtraction("r1", "unknown"), "Cancel unknown fails");
}

static void testMoonMiningRemove() {
    std::cout << "\n=== MoonMining: Remove ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    sys.startExtraction("r1", "ext1", "moon_a", "Ore1", 100.0f, 100.0f);
    sys.startExtraction("r1", "ext2", "moon_b", "Ore2", 200.0f, 200.0f);

    assertTrue(sys.removeExtraction("r1", "ext1"), "Remove ext1");
    assertTrue(sys.getExtractionCount("r1") == 1, "1 extraction left");
    assertTrue(!sys.hasExtraction("r1", "ext1"), "ext1 gone");
    assertTrue(sys.hasExtraction("r1", "ext2"), "ext2 present");
    assertTrue(!sys.removeExtraction("r1", "ext1"), "Remove nonexistent fails");
}

static void testMoonMiningClear() {
    std::cout << "\n=== MoonMining: Clear ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    sys.startExtraction("r1", "ext1", "moon_a", "Ore1", 100.0f, 100.0f);
    sys.startExtraction("r1", "ext2", "moon_b", "Ore2", 200.0f, 200.0f);

    assertTrue(sys.clearExtractions("r1"), "Clear succeeds");
    assertTrue(sys.getExtractionCount("r1") == 0, "0 extractions");
    assertTrue(!sys.hasExtraction("r1", "ext1"), "ext1 gone");
    assertTrue(!sys.hasExtraction("r1", "ext2"), "ext2 gone");
    assertTrue(sys.getTotalExtractionsStarted("r1") == 2, "Total started preserved");
}

static void testMoonMiningCountByStatus() {
    std::cout << "\n=== MoonMining: CountByStatus ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    using ES = components::MoonMiningSchedulerState::ExtractionStatus;
    sys.startExtraction("r1", "ext1", "moon_a", "Ore1", 10.0f, 100.0f);
    sys.startExtraction("r1", "ext2", "moon_b", "Ore2", 1000.0f, 200.0f);
    sys.startExtraction("r1", "ext3", "moon_c", "Ore3", 10.0f, 300.0f);

    sys.update(10.0f); // ext1 and ext3 become Ready, ext2 still extracting
    sys.fractureExtraction("r1", "ext1"); // ext1 -> Fractured

    assertTrue(sys.getCountByStatus("r1", ES::Extracting) == 1, "1 extracting");
    assertTrue(sys.getCountByStatus("r1", ES::Ready) == 1, "1 ready");
    assertTrue(sys.getCountByStatus("r1", ES::Fractured) == 1, "1 fractured");
    assertTrue(sys.getCountByStatus("r1", ES::Idle) == 0, "0 idle");
}

static void testMoonMiningFractureOnlyReady() {
    std::cout << "\n=== MoonMining: FractureOnlyReady ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);
    world.createEntity("r1");
    sys.initialize("r1");

    sys.startExtraction("r1", "ext1", "moon_a", "Ore", 1000.0f, 100.0f);
    // Still extracting — fracture should fail
    assertTrue(!sys.fractureExtraction("r1", "ext1"), "Cannot fracture while extracting");
    assertTrue(!sys.fractureExtraction("r1", "unknown"), "Cannot fracture unknown");
}

static void testMoonMiningMissing() {
    std::cout << "\n=== MoonMining: Missing ===" << std::endl;
    ecs::World world;
    systems::MoonMiningSchedulerSystem sys(&world);

    using ES = components::MoonMiningSchedulerState::ExtractionStatus;
    assertTrue(!sys.startExtraction("none", "ext1", "moon", "ore", 10.0f, 10.0f),
               "Start fails on missing");
    assertTrue(!sys.cancelExtraction("none", "ext1"), "Cancel fails on missing");
    assertTrue(!sys.fractureExtraction("none", "ext1"), "Fracture fails on missing");
    assertTrue(!sys.removeExtraction("none", "ext1"), "Remove fails on missing");
    assertTrue(!sys.clearExtractions("none"), "Clear fails on missing");
    assertTrue(!sys.setStructureId("none", "s1"), "SetStructure fails on missing");
    assertTrue(!sys.setMaxExtractions("none", 5), "SetMax fails on missing");
    assertTrue(sys.getExtractionCount("none") == 0, "0 count on missing");
    assertTrue(sys.getActiveExtractionCount("none") == 0, "0 active on missing");
    assertTrue(sys.getTotalExtractionsStarted("none") == 0, "0 started on missing");
    assertTrue(sys.getTotalFractured("none") == 0, "0 fractured on missing");
    assertTrue(approxEqual(sys.getTotalYield("none"), 0.0f), "0 yield on missing");
    assertTrue(approxEqual(sys.getTimeRemaining("none", "ext1"), 0.0f), "0 remaining on missing");
    assertTrue(!sys.hasExtraction("none", "ext1"), "No extraction on missing");
    assertTrue(sys.getStructureId("none") == "", "Empty structure on missing");
    assertTrue(sys.getCountByStatus("none", ES::Idle) == 0, "0 status on missing");
    assertTrue(sys.getExtractionStatus("none", "ext1") == "Unknown", "Unknown status on missing");
}

void run_moon_mining_scheduler_system_tests() {
    testMoonMiningInit();
    testMoonMiningConfig();
    testMoonMiningStartExtraction();
    testMoonMiningStartValidation();
    testMoonMiningMaxCap();
    testMoonMiningTickCountdown();
    testMoonMiningFracture();
    testMoonMiningCancel();
    testMoonMiningRemove();
    testMoonMiningClear();
    testMoonMiningCountByStatus();
    testMoonMiningFractureOnlyReady();
    testMoonMiningMissing();
}
