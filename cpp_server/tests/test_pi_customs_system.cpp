// Tests for: PiCustomsSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/pi_customs_system.h"

using namespace atlas;

// ==================== PiCustomsSystem Tests ====================

static void testPiCustomsInit() {
    std::cout << "\n=== PiCustoms: Init ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    assertTrue(sys.initialize("co1", "customs_0001", "sys_jita", false),
               "Init NPC-owned customs office");
    assertTrue(sys.getPendingBatches("co1") == 0, "No pending batches");
    assertTrue(sys.getCompletedBatches("co1") == 0, "No completed batches");
    assertTrue(sys.getTotalExported("co1") == 0, "Nothing exported yet");
}

static void testPiCustomsPlayerOwned() {
    std::cout << "\n=== PiCustoms: PlayerOwned ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    assertTrue(sys.initialize("co1", "customs_0002", "sys_abc", true),
               "Init player-owned customs office");
}

static void testPiCustomsQueueExport() {
    std::cout << "\n=== PiCustoms: QueueExport ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita");

    std::string bid = sys.queueExport("co1", "colony_A", "reactive_metals", 500);
    assertTrue(!bid.empty(), "Queue returns batch id");
    assertTrue(sys.getPendingBatches("co1") == 1, "1 pending batch");
}

static void testPiCustomsQueueZeroQuantity() {
    std::cout << "\n=== PiCustoms: QueueZeroQuantity ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita");

    std::string bid = sys.queueExport("co1", "colony_A", "reactive_metals", 0);
    assertTrue(bid.empty(), "Zero quantity rejected");
    assertTrue(sys.getPendingBatches("co1") == 0, "0 pending");
}

static void testPiCustomsQueueMax() {
    std::cout << "\n=== PiCustoms: QueueMax ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita");

    for (int i = 0; i < 5; i++) {
        sys.queueExport("co1", "colony_A", "reactive_metals", 100);
    }
    assertTrue(sys.getPendingBatches("co1") == 5, "5 batches at limit");

    std::string bid = sys.queueExport("co1", "colony_A", "reactive_metals", 100);
    assertTrue(bid.empty(), "6th batch rejected at capacity");
    assertTrue(sys.getPendingBatches("co1") == 5, "Still 5 pending");
}

static void testPiCustomsExportComplete() {
    std::cout << "\n=== PiCustoms: ExportComplete ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita");

    std::string bid = sys.queueExport("co1", "colony_A", "reactive_metals", 500);
    assertTrue(!bid.empty(), "Batch queued");

    sys.update(400.0f);  // well past default 300s duration
    assertTrue(sys.getCompletedBatches("co1") == 1, "1 completed");
    assertTrue(sys.getPendingBatches("co1") == 0, "0 pending");
    assertTrue(sys.getTotalExported("co1") == 500, "500 units exported");
}

static void testPiCustomsExportProgress() {
    std::cout << "\n=== PiCustoms: ExportProgress ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita");

    std::string bid = sys.queueExport("co1", "colony_A", "reactive_metals", 500);
    sys.update(150.0f);  // half of 300s
    float prog = sys.getBatchProgress("co1", bid);
    assertTrue(prog > 0.0f && prog < 1.0f, "Progress between 0 and 1");
    assertTrue(sys.getCompletedBatches("co1") == 0, "Not yet complete");
}

static void testPiCustomsCancel() {
    std::cout << "\n=== PiCustoms: Cancel ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita");

    std::string bid = sys.queueExport("co1", "colony_A", "reactive_metals", 500);
    assertTrue(sys.cancelExport("co1", bid), "Cancel succeeds");
    assertTrue(sys.getPendingBatches("co1") == 0, "0 pending after cancel");
    assertTrue(!sys.cancelExport("co1", bid), "Cancel again fails");
}

static void testPiCustomsCorpTax() {
    std::cout << "\n=== PiCustoms: CorpTax ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    world.createEntity("co1");
    sys.initialize("co1", "customs_001", "sys_jita", true);  // player owned

    // Corp member gets lower tax rate
    std::string bid_corp = sys.queueExport("co1", "colony_A", "base_metals",
                                            200, true);
    std::string bid_stranger = sys.queueExport("co1", "colony_B", "base_metals",
                                                200, false);
    assertTrue(!bid_corp.empty(), "Corp batch queued");
    assertTrue(!bid_stranger.empty(), "Stranger batch queued");
    assertTrue(sys.getPendingBatches("co1") == 2, "2 pending");
}

static void testPiCustomsMissing() {
    std::cout << "\n=== PiCustoms: Missing ===" << std::endl;
    ecs::World world;
    systems::PiCustomsSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "co_x", "sys_x"),
               "Init fails on missing entity");
    std::string bid = sys.queueExport("nonexistent", "col_A", "base_metals", 100);
    assertTrue(bid.empty(), "Queue fails on missing");
    assertTrue(!sys.cancelExport("nonexistent", "batch_1"),
               "Cancel fails on missing");
    assertTrue(sys.getPendingBatches("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getCompletedBatches("nonexistent") == 0, "0 on missing");
    assertTrue(sys.getTotalExported("nonexistent") == 0, "0 on missing");
}

void run_pi_customs_system_tests() {
    testPiCustomsInit();
    testPiCustomsPlayerOwned();
    testPiCustomsQueueExport();
    testPiCustomsQueueZeroQuantity();
    testPiCustomsQueueMax();
    testPiCustomsExportComplete();
    testPiCustomsExportProgress();
    testPiCustomsCancel();
    testPiCustomsCorpTax();
    testPiCustomsMissing();
}
