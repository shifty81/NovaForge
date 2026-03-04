// Tests for: Inventory System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/inventory_system.h"

using namespace atlas;

// ==================== Inventory System Tests ====================

static void testInventoryAddItem() {
    std::cout << "\n=== Inventory Add Item ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 100.0f;

    bool added = invSys.addItem("ship1", "stellium", "Stellium", "ore", 10, 1.0f);
    assertTrue(added, "Item added successfully");
    assertTrue(inv->items.size() == 1, "One item stack in inventory");
    assertTrue(inv->items[0].quantity == 10, "Quantity is 10");
    assertTrue(approxEqual(inv->usedCapacity(), 10.0f), "Used capacity is 10 m3");

    // Stack with existing
    added = invSys.addItem("ship1", "stellium", "Stellium", "ore", 5, 1.0f);
    assertTrue(added, "Stacked item added");
    assertTrue(inv->items.size() == 1, "Still one stack after stacking");
    assertTrue(inv->items[0].quantity == 15, "Quantity is 15 after stacking");
}

static void testInventoryCapacityLimit() {
    std::cout << "\n=== Inventory Capacity Limit ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 50.0f;

    bool added = invSys.addItem("ship1", "ore", "Ferrite", "ore", 40, 1.0f);
    assertTrue(added, "40 m3 fits in 50 m3 hold");

    added = invSys.addItem("ship1", "big_item", "Big Module", "module", 1, 20.0f);
    assertTrue(!added, "20 m3 item rejected (only 10 m3 free)");
    assertTrue(approxEqual(inv->freeCapacity(), 10.0f), "Free capacity is 10 m3");
}

static void testInventoryRemoveItem() {
    std::cout << "\n=== Inventory Remove Item ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;

    invSys.addItem("ship1", "ammo", "Hybrid Charges", "ammo", 100, 0.01f);

    int removed = invSys.removeItem("ship1", "ammo", 30);
    assertTrue(removed == 30, "Removed 30 units");
    assertTrue(invSys.getItemCount("ship1", "ammo") == 70, "70 remaining");

    removed = invSys.removeItem("ship1", "ammo", 200);
    assertTrue(removed == 70, "Removed only 70 (all available)");
    assertTrue(inv->items.empty(), "Item stack removed when depleted");
}

static void testInventoryTransfer() {
    std::cout << "\n=== Inventory Transfer ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship1 = world.createEntity("ship1");
    auto* inv1 = addComp<components::Inventory>(ship1);
    inv1->max_capacity = 400.0f;

    auto* ship2 = world.createEntity("ship2");
    auto* inv2 = addComp<components::Inventory>(ship2);
    inv2->max_capacity = 400.0f;

    invSys.addItem("ship1", "salvage", "Armor Plates", "salvage", 20, 2.0f);

    bool transferred = invSys.transferItem("ship1", "ship2", "salvage", 10);
    assertTrue(transferred, "Transfer succeeded");
    assertTrue(invSys.getItemCount("ship1", "salvage") == 10, "Source has 10 left");
    assertTrue(invSys.getItemCount("ship2", "salvage") == 10, "Destination has 10");

    // Transfer fails if source lacks quantity
    transferred = invSys.transferItem("ship1", "ship2", "nonexistent", 5);
    assertTrue(!transferred, "Transfer fails for missing item");
}

static void testInventoryHasItem() {
    std::cout << "\n=== Inventory HasItem ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Inventory>(ship);

    invSys.addItem("ship1", "dogtag", "Pirate Dogtag", "commodity", 5, 0.1f);

    assertTrue(invSys.hasItem("ship1", "dogtag", 3), "Has 3 dogtags (has 5)");
    assertTrue(invSys.hasItem("ship1", "dogtag", 5), "Has exactly 5 dogtags");
    assertTrue(!invSys.hasItem("ship1", "dogtag", 6), "Does not have 6 dogtags");
    assertTrue(!invSys.hasItem("ship1", "nope"), "Does not have nonexistent item");
}


void run_inventory_system_tests() {
    testInventoryAddItem();
    testInventoryCapacityLimit();
    testInventoryRemoveItem();
    testInventoryTransfer();
    testInventoryHasItem();
}
