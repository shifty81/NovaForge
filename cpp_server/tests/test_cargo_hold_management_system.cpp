// Tests for: Cargo Hold Management System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/cargo_hold_management_system.h"

using namespace atlas;

// ==================== Cargo Hold Management System Tests ====================

static void testCargoHoldCreate() {
    std::cout << "\n=== CargoHold: Create ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 500.0f), "Init succeeds");
    assertTrue(sys.getItemCount("ship1") == 0, "0 items");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 0.0f), "0 used volume");
    assertTrue(approxEqual(sys.getMaxVolume("ship1"), 500.0f), "500 max volume");
    assertTrue(approxEqual(sys.getFreeVolume("ship1"), 500.0f), "500 free volume");
    assertTrue(sys.getTotalJettisoned("ship1") == 0, "0 jettisoned");
}

static void testCargoHoldAddAndStack() {
    std::cout << "\n=== CargoHold: AddAndStack ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    assertTrue(sys.addItem("ship1", "tritanium", 100, 1.0f), "Add tritanium");
    assertTrue(sys.getItemCount("ship1") == 1, "1 item type");
    assertTrue(sys.getItemQuantity("ship1", "tritanium") == 100, "100 tritanium");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 100.0f), "100 used");

    // Stack with same item
    assertTrue(sys.addItem("ship1", "tritanium", 50, 1.0f), "Stack tritanium");
    assertTrue(sys.getItemCount("ship1") == 1, "Still 1 item type");
    assertTrue(sys.getItemQuantity("ship1", "tritanium") == 150, "150 tritanium");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 150.0f), "150 used");

    // Add different item
    assertTrue(sys.addItem("ship1", "pyerite", 200, 2.0f), "Add pyerite");
    assertTrue(sys.getItemCount("ship1") == 2, "2 item types");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 550.0f), "550 used (150 + 400)");
}

static void testCargoHoldCapacityLimit() {
    std::cout << "\n=== CargoHold: CapacityLimit ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f);

    assertTrue(sys.addItem("ship1", "tritanium", 50, 1.0f), "Add 50 trit");
    assertTrue(!sys.addItem("ship1", "pyerite", 60, 1.0f), "Overflow rejected");
    assertTrue(sys.getItemCount("ship1") == 1, "Still 1 item");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 50.0f), "50 used after reject");

    // Exact fill
    assertTrue(sys.addItem("ship1", "pyerite", 50, 1.0f), "Exact fill accepted");
    assertTrue(approxEqual(sys.getFreeVolume("ship1"), 0.0f), "0 free");
}

static void testCargoHoldRemoveItem() {
    std::cout << "\n=== CargoHold: RemoveItem ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    sys.addItem("ship1", "tritanium", 100, 1.0f);
    assertTrue(sys.removeItem("ship1", "tritanium", 30), "Remove 30");
    assertTrue(sys.getItemQuantity("ship1", "tritanium") == 70, "70 remaining");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 70.0f), "70 used");

    // Remove all
    assertTrue(sys.removeItem("ship1", "tritanium", 70), "Remove all");
    assertTrue(sys.getItemCount("ship1") == 0, "0 items after remove all");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 0.0f), "0 used");

    // Can't remove more than available
    sys.addItem("ship1", "pyerite", 10, 1.0f);
    assertTrue(!sys.removeItem("ship1", "pyerite", 20), "Can't remove more than available");
}

static void testCargoHoldJettison() {
    std::cout << "\n=== CargoHold: Jettison ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    sys.addItem("ship1", "tritanium", 100, 1.0f);
    sys.addItem("ship1", "pyerite", 50, 2.0f);

    assertTrue(sys.jettisonItem("ship1", "tritanium"), "Jettison tritanium");
    assertTrue(sys.getItemCount("ship1") == 1, "1 item after jettison");
    assertTrue(sys.getItemQuantity("ship1", "tritanium") == 0, "0 tritanium");
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 100.0f), "100 used (pyerite)");
    assertTrue(sys.getTotalJettisoned("ship1") == 1, "1 jettisoned");

    // Jettison non-existent item fails
    assertTrue(!sys.jettisonItem("ship1", "mexallon"), "Jettison non-existent fails");
}

static void testCargoHoldUpdateRecalcVolume() {
    std::cout << "\n=== CargoHold: UpdateRecalcVolume ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f);

    sys.addItem("ship1", "tritanium", 100, 1.5f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getUsedVolume("ship1"), 150.0f), "Volume recalculated correctly");
}

static void testCargoHoldSetMaxVolume() {
    std::cout << "\n=== CargoHold: SetMaxVolume ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 500.0f);

    assertTrue(sys.setMaxVolume("ship1", 200.0f), "Set max volume");
    assertTrue(approxEqual(sys.getMaxVolume("ship1"), 200.0f), "Max is 200");

    // Negative clamped to 0
    assertTrue(sys.setMaxVolume("ship1", -100.0f), "Negative clamped");
    assertTrue(approxEqual(sys.getMaxVolume("ship1"), 0.0f), "Max is 0");
}

static void testCargoHoldMissing() {
    std::cout << "\n=== CargoHold: Missing ===" << std::endl;
    ecs::World world;
    systems::CargoHoldManagementSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 100.0f), "Init fails on missing");
    assertTrue(!sys.addItem("nonexistent", "t", 1, 1.0f), "Add fails on missing");
    assertTrue(!sys.removeItem("nonexistent", "t", 1), "Remove fails on missing");
    assertTrue(!sys.jettisonItem("nonexistent", "t"), "Jettison fails on missing");
    assertTrue(sys.getItemCount("nonexistent") == 0, "0 items on missing");
    assertTrue(approxEqual(sys.getUsedVolume("nonexistent"), 0.0f), "0 volume on missing");
    assertTrue(approxEqual(sys.getFreeVolume("nonexistent"), 0.0f), "0 free on missing");
    assertTrue(approxEqual(sys.getMaxVolume("nonexistent"), 0.0f), "0 max on missing");
    assertTrue(sys.getTotalJettisoned("nonexistent") == 0, "0 jettisoned on missing");
}

void run_cargo_hold_management_system_tests() {
    testCargoHoldCreate();
    testCargoHoldAddAndStack();
    testCargoHoldCapacityLimit();
    testCargoHoldRemoveItem();
    testCargoHoldJettison();
    testCargoHoldUpdateRecalcVolume();
    testCargoHoldSetMaxVolume();
    testCargoHoldMissing();
}
