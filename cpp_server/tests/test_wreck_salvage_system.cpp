// Tests for: WreckSalvageSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "pcg/salvage_system.h"
#include "systems/wreck_salvage_system.h"

using namespace atlas;

// ==================== WreckSalvageSystem Tests ====================

static void testWreckCreate() {
    std::cout << "\n=== Wreck Create ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("dead_ship_1", 100.0f, 0.0f, 200.0f, 600.0f);
    assertTrue(!wreck_id.empty(), "Wreck created with valid id");

    auto* entity = world.getEntity(wreck_id);
    assertTrue(entity != nullptr, "Wreck entity exists");

    auto* wreck = entity->getComponent<components::Wreck>();
    assertTrue(wreck != nullptr, "Wreck component attached");
    assertTrue(wreck->source_entity_id == "dead_ship_1", "Source entity id correct");
    assertTrue(approxEqual(wreck->lifetime_remaining, 600.0f), "Lifetime is correct");
    assertTrue(!wreck->salvaged, "Not yet salvaged");
}

static void testWreckLifetimeDecay() {
    std::cout << "\n=== Wreck Lifetime Decay ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    wreckSys.createWreck("ship1", 0, 0, 0, 10.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 1, "One active wreck");

    wreckSys.update(5.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 1, "Wreck still active after 5s");

    wreckSys.update(6.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 0, "Wreck despawned after expiry");
}

static void testSalvageWreckInRange() {
    std::cout << "\n=== Salvage Wreck In Range ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 100.0f, 0.0f, 0.0f);

    // Add loot to wreck
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck_inv = wreck_entity->getComponent<components::Inventory>();
    components::Inventory::Item loot;
    loot.item_id = "scrap_1";
    loot.name = "Metal Scraps";
    loot.type = "salvage";
    loot.quantity = 5;
    loot.volume = 1.0f;
    wreck_inv->items.push_back(loot);

    // Create player near the wreck
    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 110.0f;
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 1000.0f;

    bool ok = wreckSys.salvageWreck("player_1", wreck_id, 2500.0f);
    assertTrue(ok, "Salvage succeeds when in range");

    assertTrue(inv->items.size() == 1, "Player received 1 item stack");
    assertTrue(inv->items[0].name == "Metal Scraps", "Correct item transferred");
    assertTrue(inv->items[0].quantity == 5, "Correct quantity transferred");
}

static void testSalvageWreckOutOfRange() {
    std::cout << "\n=== Salvage Wreck Out Of Range ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);

    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 99999.0f;
    addComp<components::Inventory>(player);

    bool ok = wreckSys.salvageWreck("player_1", wreck_id, 2500.0f);
    assertTrue(!ok, "Salvage fails when out of range");
}

static void testSalvageAlreadySalvaged() {
    std::cout << "\n=== Salvage Already Salvaged ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);
    addComp<components::Inventory>(player);

    wreckSys.salvageWreck("player_1", wreck_id, 5000.0f);
    bool again = wreckSys.salvageWreck("player_1", wreck_id, 5000.0f);
    assertTrue(!again, "Cannot salvage same wreck twice");
}

static void testWreckActiveCount() {
    std::cout << "\n=== Wreck Active Count ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    wreckSys.createWreck("s1", 0, 0, 0);
    wreckSys.createWreck("s2", 100, 0, 0);
    wreckSys.createWreck("s3", 200, 0, 0);
    assertTrue(wreckSys.getActiveWreckCount() == 3, "Three active wrecks");

    // Salvage one
    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);
    addComp<components::Inventory>(player);

    auto entities = world.getAllEntities();
    std::string first_wreck;
    for (auto* e : entities) {
        if (e->getComponent<components::Wreck>()) {
            first_wreck = e->getId();
            break;
        }
    }
    wreckSys.salvageWreck("player_1", first_wreck, 999999.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 2, "Two active after one salvaged");
}

static void testWreckHasInventory() {
    std::cout << "\n=== Wreck Has Inventory ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);
    auto* entity = world.getEntity(wreck_id);
    auto* inv = entity->getComponent<components::Inventory>();
    assertTrue(inv != nullptr, "Wreck has Inventory component");
    assertTrue(approxEqual(inv->max_capacity, 500.0f), "Wreck cargo capacity is 500 m3");
}


void run_wreck_salvage_system_tests() {
    testWreckCreate();
    testWreckLifetimeDecay();
    testSalvageWreckInRange();
    testSalvageWreckOutOfRange();
    testSalvageAlreadySalvaged();
    testWreckActiveCount();
    testWreckHasInventory();
}
