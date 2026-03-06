// Tests for: CargoManifestSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/cargo_manifest_system.h"

using namespace atlas;

// ==================== CargoManifestSystem Tests ====================

static void testCargoAddItem() {
    std::cout << "\n=== CargoManifest: Add Item ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    assertTrue(sys.addItem("ship_1", "ammo_1", "Antimatter S", "ammo", 100, 0.01), "Add ammo succeeds");
    assertTrue(sys.getItemQuantity("ship_1", "ammo_1") == 100, "Ammo quantity is 100");
    assertTrue(sys.getItemCount("ship_1") == 1, "1 item type in cargo");
    assertTrue(approxEqual(static_cast<float>(sys.getGeneralUsed("ship_1")), 1.0f), "1.0 m³ used");
}

static void testCargoAddOreToOreHold() {
    std::cout << "\n=== CargoManifest: Add Ore to Ore Hold ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* c = addComp<components::CargoManifest>(e);
    c->ore_hold_capacity = 5000.0;

    assertTrue(sys.addItem("ship_1", "veldspar", "Veldspar", "ore", 100, 10.0), "Add ore succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getOreHoldUsed("ship_1")), 1000.0f), "Ore hold used 1000");
    assertTrue(approxEqual(static_cast<float>(sys.getGeneralUsed("ship_1")), 0.0f), "General hold empty");
}

static void testCargoOreToGeneralWhenNoOreHold() {
    std::cout << "\n=== CargoManifest: Ore to General When No Ore Hold ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);
    // Default ore_hold_capacity is 0 (no ore hold)

    assertTrue(sys.addItem("ship_1", "veldspar", "Veldspar", "ore", 10, 10.0), "Add ore to general");
    assertTrue(approxEqual(static_cast<float>(sys.getGeneralUsed("ship_1")), 100.0f), "100 m³ in general");
    assertTrue(approxEqual(static_cast<float>(sys.getOreHoldUsed("ship_1")), 0.0f), "Ore hold still 0");
}

static void testCargoCapacityEnforcement() {
    std::cout << "\n=== CargoManifest: Capacity Enforcement ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* c = addComp<components::CargoManifest>(e);
    c->general_capacity = 100.0;

    assertTrue(sys.addItem("ship_1", "mod_1", "Shield Booster", "module", 1, 50.0), "First module fits");
    assertTrue(!sys.addItem("ship_1", "mod_2", "Armor Plate", "module", 1, 60.0), "Second module rejected");
    assertTrue(sys.getItemCount("ship_1") == 1, "Still only 1 item");
}

static void testCargoRemoveItem() {
    std::cout << "\n=== CargoManifest: Remove Item ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    sys.addItem("ship_1", "ammo_1", "Antimatter S", "ammo", 100, 0.01);
    assertTrue(sys.removeItem("ship_1", "ammo_1", 40), "Remove 40 ammo succeeds");
    assertTrue(sys.getItemQuantity("ship_1", "ammo_1") == 60, "60 remaining");
    assertTrue(approxEqual(static_cast<float>(sys.getGeneralUsed("ship_1")), 0.6f), "0.6 m³ used");
}

static void testCargoRemoveAll() {
    std::cout << "\n=== CargoManifest: Remove All ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    sys.addItem("ship_1", "ammo_1", "Antimatter S", "ammo", 50, 0.01);
    assertTrue(sys.removeItem("ship_1", "ammo_1", 50), "Remove all ammo");
    assertTrue(sys.getItemQuantity("ship_1", "ammo_1") == 0, "No ammo left");
    assertTrue(sys.getItemCount("ship_1") == 0, "Item entry removed");
}

static void testCargoRemoveTooMany() {
    std::cout << "\n=== CargoManifest: Remove Too Many ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    sys.addItem("ship_1", "ammo_1", "Antimatter S", "ammo", 50, 0.01);
    assertTrue(!sys.removeItem("ship_1", "ammo_1", 100), "Remove rejected");
    assertTrue(sys.getItemQuantity("ship_1", "ammo_1") == 50, "Quantity unchanged");
}

static void testCargoJettison() {
    std::cout << "\n=== CargoManifest: Jettison ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    sys.addItem("ship_1", "salvage_1", "Scrap Metal", "salvage", 20, 5.0);
    assertTrue(sys.jettisonItem("ship_1", "salvage_1", 10), "Jettison 10 succeeds");
    assertTrue(sys.getItemQuantity("ship_1", "salvage_1") == 10, "10 remaining after jettison");
}

static void testCargoTransfer() {
    std::cout << "\n=== CargoManifest: Transfer ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e1 = world.createEntity("ship_1");
    auto* e2 = world.createEntity("ship_2");
    addComp<components::CargoManifest>(e1);
    addComp<components::CargoManifest>(e2);

    sys.addItem("ship_1", "minerals", "Tritanium", "mineral", 100, 0.01);
    assertTrue(sys.transferItem("ship_1", "ship_2", "minerals", 50), "Transfer 50 succeeds");
    assertTrue(sys.getItemQuantity("ship_1", "minerals") == 50, "50 left in source");
    assertTrue(sys.getItemQuantity("ship_2", "minerals") == 50, "50 in destination");
}

static void testCargoTransferNoSpace() {
    std::cout << "\n=== CargoManifest: Transfer No Space ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e1 = world.createEntity("ship_1");
    auto* e2 = world.createEntity("ship_2");
    addComp<components::CargoManifest>(e1);
    auto* c2 = addComp<components::CargoManifest>(e2);
    c2->general_capacity = 10.0;

    sys.addItem("ship_1", "mod_1", "Plate", "module", 100, 1.0);
    assertTrue(!sys.transferItem("ship_1", "ship_2", "mod_1", 50), "Transfer rejected (no space)");
    assertTrue(sys.getItemQuantity("ship_1", "mod_1") == 100, "Source unchanged");
    assertTrue(sys.getItemQuantity("ship_2", "mod_1") == 0, "Dest still empty");
}

static void testCargoStackMerge() {
    std::cout << "\n=== CargoManifest: Stack Merge ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    sys.addItem("ship_1", "ammo_1", "Antimatter S", "ammo", 50, 0.01);
    sys.addItem("ship_1", "ammo_1", "Antimatter S", "ammo", 30, 0.01);

    assertTrue(sys.getItemQuantity("ship_1", "ammo_1") == 80, "Stacks merged to 80");
    assertTrue(sys.getItemCount("ship_1") == 1, "Still 1 item type");
    assertTrue(approxEqual(static_cast<float>(sys.getGeneralUsed("ship_1")), 0.8f), "0.8 m³ used");
}

static void testCargoHasSpace() {
    std::cout << "\n=== CargoManifest: Has Space ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* c = addComp<components::CargoManifest>(e);
    c->general_capacity = 100.0;
    c->ore_hold_capacity = 500.0;

    assertTrue(sys.hasSpace("ship_1", "module", 50.0), "50 m³ fits in general");
    assertTrue(!sys.hasSpace("ship_1", "module", 200.0), "200 m³ doesn't fit");
    assertTrue(sys.hasSpace("ship_1", "ore", 400.0), "400 m³ fits in ore hold");
    assertTrue(!sys.hasSpace("ship_1", "ore", 600.0), "600 m³ doesn't fit ore hold");
}

static void testCargoTotalVolume() {
    std::cout << "\n=== CargoManifest: Total Volume ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    auto* c = addComp<components::CargoManifest>(e);
    c->ore_hold_capacity = 5000.0;

    sys.addItem("ship_1", "mod_1", "Shield Booster", "module", 1, 50.0);
    sys.addItem("ship_1", "veldspar", "Veldspar", "ore", 100, 10.0);

    assertTrue(approxEqual(static_cast<float>(sys.getTotalVolumeUsed("ship_1")), 1050.0f), "Total volume 1050");
}

static void testCargoMissingEntity() {
    std::cout << "\n=== CargoManifest: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    assertTrue(!sys.addItem("nonexistent", "a", "A", "ammo", 1, 1.0), "Add fails on missing");
    assertTrue(!sys.removeItem("nonexistent", "a", 1), "Remove fails on missing");
    assertTrue(sys.getItemQuantity("nonexistent", "a") == 0, "0 quantity on missing");
    assertTrue(sys.getItemCount("nonexistent") == 0, "0 items on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getGeneralUsed("nonexistent")), 0.0f), "0 general used");
    assertTrue(approxEqual(static_cast<float>(sys.getOreHoldUsed("nonexistent")), 0.0f), "0 ore hold used");
    assertTrue(!sys.hasSpace("nonexistent", "ammo", 1.0), "No space on missing");
}

static void testCargoInvalidInput() {
    std::cout << "\n=== CargoManifest: Invalid Input ===" << std::endl;
    ecs::World world;
    systems::CargoManifestSystem sys(&world);

    auto* e = world.createEntity("ship_1");
    addComp<components::CargoManifest>(e);

    assertTrue(!sys.addItem("ship_1", "a", "A", "ammo", -1, 1.0), "Negative quantity rejected");
    assertTrue(!sys.addItem("ship_1", "a", "A", "ammo", 1, -1.0), "Negative volume rejected");
    assertTrue(!sys.addItem("ship_1", "a", "A", "ammo", 0, 1.0), "Zero quantity rejected");
    assertTrue(!sys.removeItem("ship_1", "a", -1), "Negative remove rejected");
}

void run_cargo_manifest_system_tests() {
    testCargoAddItem();
    testCargoAddOreToOreHold();
    testCargoOreToGeneralWhenNoOreHold();
    testCargoCapacityEnforcement();
    testCargoRemoveItem();
    testCargoRemoveAll();
    testCargoRemoveTooMany();
    testCargoJettison();
    testCargoTransfer();
    testCargoTransferNoSpace();
    testCargoStackMerge();
    testCargoHasSpace();
    testCargoTotalVolume();
    testCargoMissingEntity();
    testCargoInvalidInput();
}
