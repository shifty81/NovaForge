// Tests for: LootContainerSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/loot_container_system.h"

using namespace atlas;

// ==================== LootContainerSystem Tests ====================

static void testLootAddItem() {
    std::cout << "\n=== LootContainer: Add Item ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    assertTrue(sys.addItem("wreck_1", "mod_1", "Shield Booster", "Module", 1, 5.0f, 50000.0f),
               "Add item succeeds");
    assertTrue(sys.getItemCount("wreck_1") == 1, "1 item in container");
    assertTrue(approxEqual(sys.getTotalValue("wreck_1"), 50000.0f), "Value is 50000");
    assertTrue(approxEqual(sys.getTotalVolume("wreck_1"), 5.0f), "Volume is 5.0");
}

static void testLootRemoveItem() {
    std::cout << "\n=== LootContainer: Remove Item ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    sys.addItem("wreck_1", "ammo_1", "Antimatter", "Ammo", 100, 0.01f, 10.0f);
    assertTrue(sys.removeItem("wreck_1", "ammo_1", 50), "Remove 50 succeeds");
    assertTrue(sys.getItemCount("wreck_1") == 1, "Item still exists (50 remaining)");
    assertTrue(sys.getTotalLooted("wreck_1") == 50, "50 looted");
    assertTrue(approxEqual(sys.getTotalValue("wreck_1"), 500.0f), "Value reduced");
}

static void testLootRemoveAllQuantity() {
    std::cout << "\n=== LootContainer: Remove All Quantity ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    sys.addItem("wreck_1", "ore_1", "Veldspar", "Ore", 10, 0.1f, 5.0f);
    assertTrue(sys.removeItem("wreck_1", "ore_1", 10), "Remove all succeeds");
    assertTrue(sys.getItemCount("wreck_1") == 0, "Item removed entirely");
}

static void testLootItemStacking() {
    std::cout << "\n=== LootContainer: Item Stacking ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    sys.addItem("wreck_1", "ore_1", "Veldspar", "Ore", 100, 0.1f, 5.0f);
    sys.addItem("wreck_1", "ore_1", "Veldspar", "Ore", 50, 0.1f, 5.0f);
    assertTrue(sys.getItemCount("wreck_1") == 1, "Stacked into 1 entry");
    assertTrue(approxEqual(sys.getTotalVolume("wreck_1"), 15.0f), "Volume is 150 * 0.1");
}

static void testLootExpiry() {
    std::cout << "\n=== LootContainer: Expiry Timer ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";
    lc->expiry_duration = 10.0f;
    lc->time_remaining = 10.0f;

    sys.addItem("wreck_1", "mod_1", "Armor Plate", "Module", 1, 10.0f, 20000.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("wreck_1"), 10.0f), "10s remaining");

    sys.update(5.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("wreck_1"), 5.0f), "5s remaining");

    sys.update(6.0f);
    assertTrue(approxEqual(sys.getTimeRemaining("wreck_1"), 0.0f), "Expired");
    assertTrue(!lc->active, "Container deactivated on expiry");
}

static void testLootAccessControl() {
    std::cout << "\n=== LootContainer: Access Control ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    assertTrue(sys.isAccessible("wreck_1", "player_1"), "Owner can access");
    assertTrue(!sys.isAccessible("wreck_1", "player_2"), "Non-owner cannot access");
}

static void testLootAbandon() {
    std::cout << "\n=== LootContainer: Abandon ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    assertTrue(sys.abandonContainer("wreck_1"), "Abandon succeeds");
    assertTrue(sys.isAccessible("wreck_1", "anyone"), "Anyone can access abandoned");
    assertTrue(sys.isAccessible("wreck_1", "player_2"), "Player 2 can access");
}

static void testLootLock() {
    std::cout << "\n=== LootContainer: Lock/Unlock ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    sys.addItem("wreck_1", "mod_1", "Gun", "Module", 1, 5.0f, 1000.0f);
    assertTrue(sys.lockContainer("wreck_1", true), "Lock succeeds");
    assertTrue(!sys.addItem("wreck_1", "mod_2", "Shield", "Module", 1, 5.0f, 2000.0f),
               "Add fails when locked");
    assertTrue(!sys.removeItem("wreck_1", "mod_1", 1), "Remove fails when locked");
    assertTrue(!sys.isAccessible("wreck_1", "player_1"), "Not accessible when locked");

    assertTrue(sys.lockContainer("wreck_1", false), "Unlock succeeds");
    assertTrue(sys.isAccessible("wreck_1", "player_1"), "Accessible again");
}

static void testLootMaxItems() {
    std::cout << "\n=== LootContainer: Max Items ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";
    lc->max_items = 3;

    sys.addItem("wreck_1", "a", "A", "Module", 1, 1.0f, 100.0f);
    sys.addItem("wreck_1", "b", "B", "Module", 1, 1.0f, 200.0f);
    sys.addItem("wreck_1", "c", "C", "Module", 1, 1.0f, 300.0f);
    assertTrue(!sys.addItem("wreck_1", "d", "D", "Module", 1, 1.0f, 400.0f), "Max items enforced");
    assertTrue(sys.getItemCount("wreck_1") == 3, "Still 3 items");
}

static void testLootMissingEntity() {
    std::cout << "\n=== LootContainer: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    assertTrue(!sys.addItem("nope", "a", "A", "M", 1, 1.0f, 1.0f), "Add fails");
    assertTrue(!sys.removeItem("nope", "a", 1), "Remove fails");
    assertTrue(sys.getItemCount("nope") == 0, "0 items");
    assertTrue(approxEqual(sys.getTotalValue("nope"), 0.0f), "0 value");
    assertTrue(approxEqual(sys.getTimeRemaining("nope"), 0.0f), "0 time");
    assertTrue(sys.getTotalLooted("nope") == 0, "0 looted");
    assertTrue(!sys.isAccessible("nope", "anyone"), "Not accessible");
}

static void testLootSetOwner() {
    std::cout << "\n=== LootContainer: Set Owner ===" << std::endl;
    ecs::World world;
    systems::LootContainerSystem sys(&world);

    auto* e = world.createEntity("wreck_1");
    auto* lc = addComp<components::LootContainer>(e);
    lc->owner_id = "player_1";

    assertTrue(sys.isAccessible("wreck_1", "player_1"), "player_1 has access");
    assertTrue(!sys.isAccessible("wreck_1", "player_2"), "player_2 no access");

    assertTrue(sys.setOwner("wreck_1", "player_2"), "Change owner succeeds");
    assertTrue(!sys.isAccessible("wreck_1", "player_1"), "player_1 lost access");
    assertTrue(sys.isAccessible("wreck_1", "player_2"), "player_2 now has access");
}

void run_loot_container_system_tests() {
    testLootAddItem();
    testLootRemoveItem();
    testLootRemoveAllQuantity();
    testLootItemStacking();
    testLootExpiry();
    testLootAccessControl();
    testLootAbandon();
    testLootLock();
    testLootMaxItems();
    testLootMissingEntity();
    testLootSetOwner();
}
