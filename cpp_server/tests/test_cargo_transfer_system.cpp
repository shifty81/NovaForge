// Tests for: CargoTransfer System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/cargo_transfer_system.h"

using namespace atlas;

// ==================== CargoTransfer System Tests ====================

static void testCargoTransferCreate() {
    std::cout << "\n=== CargoTransfer: Create ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeTransfers("ship1", 3), "Init transfers succeeds");
    assertTrue(sys.getActiveTransferCount("ship1") == 0, "No active transfers initially");
    assertTrue(sys.getTotalCompleted("ship1") == 0, "0 completed initially");
    assertTrue(approxEqual(sys.getTotalUnitsTransferred("ship1"), 0.0f), "0 units transferred");
}

static void testCargoTransferStart() {
    std::cout << "\n=== CargoTransfer: Start ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    world.createEntity("station1");
    sys.initializeTransfers("ship1", 3);

    assertTrue(sys.startTransfer("ship1", "station1", "Veldspar", 500.0f), "Start transfer");
    assertTrue(sys.getActiveTransferCount("ship1") == 1, "1 active transfer");
    assertTrue(approxEqual(sys.getTransferProgress("ship1", 0), 0.0f), "0% progress initially");
}

static void testCargoTransferCompletion() {
    std::cout << "\n=== CargoTransfer: Completion ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    world.createEntity("station1");
    sys.initializeTransfers("ship1");

    sys.startTransfer("ship1", "station1", "Tritanium", 100.0f, 100.0f);
    // 100 units at 100/sec = 1 second
    for (int i = 0; i < 10; ++i) sys.update(0.1f);

    assertTrue(sys.getActiveTransferCount("ship1") == 0, "Transfer completed");
    assertTrue(sys.getTotalCompleted("ship1") == 1, "1 completed");
    assertTrue(approxEqual(sys.getTotalUnitsTransferred("ship1"), 100.0f), "100 units transferred");
}

static void testCargoTransferProgress() {
    std::cout << "\n=== CargoTransfer: Progress ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    world.createEntity("station1");
    sys.initializeTransfers("ship1");

    sys.startTransfer("ship1", "station1", "Ore", 200.0f, 100.0f);
    // 0.5 seconds = 50 units out of 200 = 25%
    for (int i = 0; i < 5; ++i) sys.update(0.1f);

    float progress = sys.getTransferProgress("ship1", 0);
    assertTrue(progress > 0.2f && progress < 0.3f, "~25% progress after 0.5s");
}

static void testCargoTransferMaxConcurrent() {
    std::cout << "\n=== CargoTransfer: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    world.createEntity("s1");
    world.createEntity("s2");
    world.createEntity("s3");
    sys.initializeTransfers("ship1", 2);

    assertTrue(sys.startTransfer("ship1", "s1", "Ore", 100.0f), "First transfer OK");
    assertTrue(sys.startTransfer("ship1", "s2", "Ice", 100.0f), "Second transfer OK");
    assertTrue(!sys.startTransfer("ship1", "s3", "Gas", 100.0f), "Third rejected at max=2");
}

static void testCargoTransferCancel() {
    std::cout << "\n=== CargoTransfer: Cancel ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    world.createEntity("station1");
    sys.initializeTransfers("ship1");

    sys.startTransfer("ship1", "station1", "Ore", 500.0f);
    assertTrue(sys.cancelAllTransfers("ship1"), "Cancel succeeds");
    assertTrue(sys.getActiveTransferCount("ship1") == 0, "0 after cancel");
}

static void testCargoTransferInvalidInputs() {
    std::cout << "\n=== CargoTransfer: InvalidInputs ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTransfers("ship1");

    assertTrue(!sys.startTransfer("ship1", "", "Ore", 100.0f), "Empty target rejected");
    assertTrue(!sys.startTransfer("ship1", "nonexistent", "Ore", 100.0f), "Missing target rejected");
    assertTrue(!sys.startTransfer("ship1", "ship1", "", 100.0f), "Empty item rejected");
    assertTrue(!sys.startTransfer("ship1", "ship1", "Ore", 0.0f), "Zero amount rejected");
    assertTrue(!sys.startTransfer("ship1", "ship1", "Ore", -10.0f), "Negative amount rejected");
}

static void testCargoTransferDuplicateInit() {
    std::cout << "\n=== CargoTransfer: DuplicateInit ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeTransfers("ship1"), "First init succeeds");
    assertTrue(!sys.initializeTransfers("ship1"), "Duplicate init rejected");
}

static void testCargoTransferMissing() {
    std::cout << "\n=== CargoTransfer: Missing ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    assertTrue(!sys.initializeTransfers("nonexistent"), "Init fails on missing");
    assertTrue(!sys.startTransfer("nonexistent", "t1", "Ore", 10.0f), "Start fails on missing");
    assertTrue(sys.getActiveTransferCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(!sys.cancelAllTransfers("nonexistent"), "Cancel fails on missing");
}

static void testCargoTransferMultiple() {
    std::cout << "\n=== CargoTransfer: Multiple ===" << std::endl;
    ecs::World world;
    systems::CargoTransferSystem sys(&world);
    world.createEntity("ship1");
    world.createEntity("s1");
    world.createEntity("s2");
    sys.initializeTransfers("ship1", 5);

    sys.startTransfer("ship1", "s1", "Ore", 50.0f, 100.0f);
    sys.startTransfer("ship1", "s2", "Ice", 50.0f, 100.0f);
    // Both 50 units at 100/sec = 0.5 seconds
    for (int i = 0; i < 5; ++i) sys.update(0.1f);

    assertTrue(sys.getTotalCompleted("ship1") == 2, "Both transfers completed");
    assertTrue(approxEqual(sys.getTotalUnitsTransferred("ship1"), 100.0f), "100 total units");
}

void run_cargo_transfer_system_tests() {
    testCargoTransferCreate();
    testCargoTransferStart();
    testCargoTransferCompletion();
    testCargoTransferProgress();
    testCargoTransferMaxConcurrent();
    testCargoTransferCancel();
    testCargoTransferInvalidInputs();
    testCargoTransferDuplicateInit();
    testCargoTransferMissing();
    testCargoTransferMultiple();
}
