// Tests for: Ore Compression System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/ore_compression_system.h"

using namespace atlas;

// ==================== Ore Compression System Tests ====================

static void testOreCompressionCreate() {
    std::cout << "\n=== OreCompression: Create ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    assertTrue(sys.initialize("comp1"), "Init succeeds");
    assertTrue(sys.getOreTypeCount("comp1") == 0, "No ore types initially");
    assertTrue(sys.getCompressionState("comp1") == "Idle", "State is Idle");
    assertTrue(sys.getCurrentOre("comp1") == "", "No current ore");
    assertTrue(sys.getRawQueued("comp1") == 0, "No raw queued");
    assertTrue(sys.getCompressedProduced("comp1") == 0, "No compressed produced");
    assertTrue(sys.getTotalIscSpent("comp1") == 0.0, "No ISC spent");
    assertTrue(sys.getTotalBatchesProcessed("comp1") == 0, "No batches processed");
    assertTrue(sys.getTotalRawConsumed("comp1") == 0, "No raw consumed");
    assertTrue(sys.getTotalCompressedProduced("comp1") == 0, "No compressed total");
}

static void testOreCompressionAddOreTypes() {
    std::cout << "\n=== OreCompression: AddOreTypes ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    assertTrue(sys.addOreType("comp1", "Veldspar", 10.0f, 5.0f, 50.0), "Add Veldspar");
    assertTrue(sys.addOreType("comp1", "Scordite", 8.0f, 6.0f, 75.0), "Add Scordite");
    assertTrue(sys.addOreType("comp1", "Pyroxeres", 5.0f, 8.0f, 100.0), "Add Pyroxeres");
    assertTrue(sys.getOreTypeCount("comp1") == 3, "3 ore types");
}

static void testOreCompressionDuplicateOre() {
    std::cout << "\n=== OreCompression: DuplicateOre ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    sys.addOreType("comp1", "Veldspar", 10.0f, 5.0f, 50.0);
    assertTrue(!sys.addOreType("comp1", "Veldspar", 20.0f, 3.0f, 25.0), "Duplicate rejected");
    assertTrue(sys.getOreTypeCount("comp1") == 1, "Still 1 ore type");
}

static void testOreCompressionInvalidOre() {
    std::cout << "\n=== OreCompression: InvalidOre ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    assertTrue(!sys.addOreType("comp1", "Bad", 0.0f, 5.0f, 50.0), "Reject zero ratio");
    assertTrue(!sys.addOreType("comp1", "Bad", 10.0f, 0.0f, 50.0), "Reject zero time");
    assertTrue(!sys.addOreType("comp1", "Bad", -1.0f, 5.0f, 50.0), "Reject negative ratio");
}

static void testOreCompressionStartAndComplete() {
    std::cout << "\n=== OreCompression: StartAndComplete ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    sys.addOreType("comp1", "Veldspar", 10.0f, 5.0f, 50.0);

    assertTrue(sys.startCompression("comp1", "Veldspar", 100), "Start compression");
    assertTrue(sys.getCompressionState("comp1") == "Compressing", "State is Compressing");
    assertTrue(sys.getCurrentOre("comp1") == "Veldspar", "Current ore is Veldspar");
    assertTrue(sys.getRawQueued("comp1") == 100, "100 raw queued");

    sys.update(3.0f); // 3 of 5 seconds
    assertTrue(sys.getCompressionState("comp1") == "Compressing", "Still compressing at 3s");

    sys.update(3.0f); // 6s total, past 5s process time
    assertTrue(sys.getCompressionState("comp1") == "Complete", "Complete after timer");
    assertTrue(sys.getCompressedProduced("comp1") == 10, "100 / 10 = 10 compressed");
    assertTrue(sys.getTotalRawConsumed("comp1") == 100, "100 raw consumed");
    assertTrue(sys.getTotalCompressedProduced("comp1") == 10, "10 total compressed");
    assertTrue(sys.getTotalBatchesProcessed("comp1") == 1, "1 batch processed");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscSpent("comp1")), 50.0f), "50 ISC spent");
}

static void testOreCompressionCollect() {
    std::cout << "\n=== OreCompression: Collect ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    sys.addOreType("comp1", "Veldspar", 10.0f, 2.0f, 50.0);
    sys.startCompression("comp1", "Veldspar", 50);
    sys.update(3.0f); // Complete

    assertTrue(sys.collectCompressed("comp1"), "Collect succeeds");
    assertTrue(sys.getCompressionState("comp1") == "Idle", "State is Idle after collect");
    assertTrue(sys.getCompressedProduced("comp1") == 0, "Compressed cleared after collect");
    assertTrue(sys.getCurrentOre("comp1") == "", "No current ore after collect");
}

static void testOreCompressionCancel() {
    std::cout << "\n=== OreCompression: Cancel ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    sys.addOreType("comp1", "Veldspar", 10.0f, 5.0f, 50.0);
    sys.startCompression("comp1", "Veldspar", 100);

    assertTrue(sys.cancelCompression("comp1"), "Cancel succeeds");
    assertTrue(sys.getCompressionState("comp1") == "Idle", "State is Idle after cancel");
    assertTrue(sys.getRawQueued("comp1") == 0, "Raw cleared after cancel");
}

static void testOreCompressionCannotStartWhileCompressing() {
    std::cout << "\n=== OreCompression: CannotStartWhileCompressing ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    sys.addOreType("comp1", "Veldspar", 10.0f, 5.0f, 50.0);
    sys.addOreType("comp1", "Scordite", 8.0f, 6.0f, 75.0);
    sys.startCompression("comp1", "Veldspar", 100);
    assertTrue(!sys.startCompression("comp1", "Scordite", 50), "Cannot start while compressing");
}

static void testOreCompressionUnknownOre() {
    std::cout << "\n=== OreCompression: UnknownOre ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    assertTrue(!sys.startCompression("comp1", "FakeOre", 100), "Cannot compress unknown ore");
}

static void testOreCompressionMaxQueue() {
    std::cout << "\n=== OreCompression: MaxQueue ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    world.createEntity("comp1");
    sys.initialize("comp1");
    sys.addOreType("comp1", "Veldspar", 10.0f, 5.0f, 50.0);

    auto* entity = world.getEntity("comp1");
    auto* comp = entity->getComponent<components::OreCompression>();
    comp->max_queue = 50;

    assertTrue(!sys.startCompression("comp1", "Veldspar", 100), "Exceeds max queue");
    assertTrue(sys.startCompression("comp1", "Veldspar", 50), "At max queue");
}

static void testOreCompressionMissing() {
    std::cout << "\n=== OreCompression: Missing ===" << std::endl;
    ecs::World world;
    systems::OreCompressionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addOreType("nonexistent", "Veldspar", 10.0f, 5.0f, 50.0), "AddOre fails on missing");
    assertTrue(!sys.startCompression("nonexistent", "Veldspar", 100), "Start fails on missing");
    assertTrue(!sys.cancelCompression("nonexistent"), "Cancel fails on missing");
    assertTrue(!sys.collectCompressed("nonexistent"), "Collect fails on missing");
    assertTrue(sys.getOreTypeCount("nonexistent") == 0, "0 types on missing");
    assertTrue(sys.getCompressionState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(sys.getCurrentOre("nonexistent") == "", "Empty ore on missing");
    assertTrue(sys.getRawQueued("nonexistent") == 0, "0 raw on missing");
    assertTrue(sys.getCompressedProduced("nonexistent") == 0, "0 compressed on missing");
    assertTrue(sys.getProcessTimer("nonexistent") == 0.0f, "0 timer on missing");
    assertTrue(sys.getTotalIscSpent("nonexistent") == 0.0, "0 ISC on missing");
    assertTrue(sys.getTotalBatchesProcessed("nonexistent") == 0, "0 batches on missing");
    assertTrue(sys.getTotalRawConsumed("nonexistent") == 0, "0 raw consumed on missing");
    assertTrue(sys.getTotalCompressedProduced("nonexistent") == 0, "0 compressed on missing");
}


void run_ore_compression_system_tests() {
    testOreCompressionCreate();
    testOreCompressionAddOreTypes();
    testOreCompressionDuplicateOre();
    testOreCompressionInvalidOre();
    testOreCompressionStartAndComplete();
    testOreCompressionCollect();
    testOreCompressionCancel();
    testOreCompressionCannotStartWhileCompressing();
    testOreCompressionUnknownOre();
    testOreCompressionMaxQueue();
    testOreCompressionMissing();
}
