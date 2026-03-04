// Tests for: DroneSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "data/world_persistence.h"
#include "systems/drone_system.h"

using namespace atlas;

// ==================== DroneSystem Tests ====================

static void testDroneLaunch() {
    std::cout << "\n=== Drone Launch ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->bay_capacity = 25.0f;
    bay->max_bandwidth = 25;

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.rate_of_fire = 3.0f; d.optimal_range = 5000.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    bay->stored_drones.push_back(d);

    assertTrue(droneSys.launchDrone("player_ship", "hobgoblin"),
               "Drone launched successfully");
    assertTrue(bay->deployed_drones.size() == 1, "One drone deployed");
    assertTrue(bay->stored_drones.empty(), "Bay empty after launch");
    assertTrue(droneSys.getDeployedCount("player_ship") == 1, "getDeployedCount returns 1");
}

static void testDroneRecall() {
    std::cout << "\n=== Drone Recall ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "warrior"; d.name = "Warrior I";
    d.type = "light_combat_drone"; d.damage_type = "explosive";
    d.damage = 22.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    d.hitpoints = 38.0f; d.current_hp = 38.0f;
    bay->stored_drones.push_back(d);

    droneSys.launchDrone("player_ship", "warrior");
    assertTrue(bay->deployed_drones.size() == 1, "Drone deployed before recall");

    assertTrue(droneSys.recallDrone("player_ship", "warrior"),
               "Drone recalled successfully");
    assertTrue(bay->deployed_drones.empty(), "No deployed drones after recall");
    assertTrue(bay->stored_drones.size() == 1, "Drone back in bay");
}

static void testDroneRecallAll() {
    std::cout << "\n=== Drone Recall All ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->max_bandwidth = 25;

    // Add 3 drones
    for (int i = 0; i < 3; ++i) {
        components::DroneBay::DroneInfo d;
        d.drone_id = "drone_" + std::to_string(i);
        d.name = "Test Drone " + std::to_string(i);
        d.type = "light_combat_drone"; d.damage_type = "thermal";
        d.damage = 10.0f; d.bandwidth_use = 5; d.volume = 5.0f;
        d.hitpoints = 40.0f; d.current_hp = 40.0f;
        bay->stored_drones.push_back(d);
    }

    // Launch all 3
    droneSys.launchDrone("player_ship", "drone_0");
    droneSys.launchDrone("player_ship", "drone_1");
    droneSys.launchDrone("player_ship", "drone_2");
    assertTrue(bay->deployed_drones.size() == 3, "3 drones deployed");

    int recalled = droneSys.recallAll("player_ship");
    assertTrue(recalled == 3, "recallAll returns 3");
    assertTrue(bay->deployed_drones.empty(), "No deployed drones after recallAll");
    assertTrue(bay->stored_drones.size() == 3, "All drones back in bay");
}

static void testDroneBandwidthLimit() {
    std::cout << "\n=== Drone Bandwidth Limit ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->max_bandwidth = 10;  // Only 10 Mbit/s

    // Add two drones each using 5 bandwidth (exactly max), then a third
    for (int i = 0; i < 3; ++i) {
        components::DroneBay::DroneInfo d;
        d.drone_id = "drone_" + std::to_string(i);
        d.name = "Test Drone " + std::to_string(i);
        d.type = "light_combat_drone"; d.damage_type = "kinetic";
        d.damage = 10.0f; d.bandwidth_use = 5; d.volume = 5.0f;
        d.hitpoints = 40.0f; d.current_hp = 40.0f;
        bay->stored_drones.push_back(d);
    }

    assertTrue(droneSys.launchDrone("player_ship", "drone_0"), "First drone fits bandwidth");
    assertTrue(droneSys.launchDrone("player_ship", "drone_1"), "Second drone fits bandwidth");
    assertTrue(!droneSys.launchDrone("player_ship", "drone_2"),
               "Third drone exceeds bandwidth limit");
    assertTrue(bay->deployed_drones.size() == 2, "Only 2 drones deployed");
    assertTrue(bay->stored_drones.size() == 1, "One drone remains in bay");
}

static void testDroneCombatUpdate() {
    std::cout << "\n=== Drone Combat Update ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    // Create player ship with drone
    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* target_comp = addComp<components::Target>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.rate_of_fire = 3.0f; d.optimal_range = 5000.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f; d.bandwidth_use = 5;
    bay->stored_drones.push_back(d);
    droneSys.launchDrone("player_ship", "hobgoblin");

    // Create target NPC
    auto* npc = world.createEntity("npc_target");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;

    // Lock the target
    target_comp->locked_targets.push_back("npc_target");

    // First tick: drone fires (cooldown == 0 initially)
    droneSys.update(0.1f);
    assertTrue(hp->shield_hp < 100.0f, "Drone dealt damage to shields");
    float shield_after = hp->shield_hp;

    // Second tick: drone is on cooldown, no additional damage
    droneSys.update(0.1f);
    assertTrue(approxEqual(hp->shield_hp, shield_after),
               "Drone on cooldown, no additional damage");

    // Wait out the cooldown (3.0 seconds)
    droneSys.update(3.0f);
    // Cooldown just expired this tick; drone fires on next update
    droneSys.update(0.01f);
    assertTrue(hp->shield_hp < shield_after, "Drone fires again after cooldown");
}

static void testDroneDestroyedRemoval() {
    std::cout << "\n=== Drone Destroyed Removal ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f;
    bay->stored_drones.push_back(d);
    droneSys.launchDrone("player_ship", "hobgoblin");
    assertTrue(bay->deployed_drones.size() == 1, "Drone deployed");

    // Simulate drone being destroyed
    bay->deployed_drones[0].current_hp = 0.0f;

    droneSys.update(1.0f);
    assertTrue(bay->deployed_drones.empty(), "Destroyed drone removed from deployed list");
}

static void testSerializeDeserializeDroneBay() {
    std::cout << "\n=== Serialize/Deserialize DroneBay ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("drone_ship");
    auto* bay = addComp<components::DroneBay>(entity);
    bay->bay_capacity = 50.0f;
    bay->max_bandwidth = 50;

    // Add stored drone
    components::DroneBay::DroneInfo stored;
    stored.drone_id = "ogre"; stored.name = "Ogre I";
    stored.type = "heavy_combat_drone"; stored.damage_type = "thermal";
    stored.damage = 55.0f; stored.rate_of_fire = 6.0f;
    stored.optimal_range = 3000.0f; stored.hitpoints = 120.0f;
    stored.current_hp = 120.0f; stored.bandwidth_use = 25; stored.volume = 25.0f;
    bay->stored_drones.push_back(stored);

    // Add deployed drone
    components::DroneBay::DroneInfo deployed;
    deployed.drone_id = "hobgoblin"; deployed.name = "Hobgoblin I";
    deployed.type = "light_combat_drone"; deployed.damage_type = "thermal";
    deployed.damage = 25.0f; deployed.rate_of_fire = 3.0f;
    deployed.optimal_range = 5000.0f; deployed.hitpoints = 45.0f;
    deployed.current_hp = 30.0f; deployed.bandwidth_use = 5; deployed.volume = 5.0f;
    bay->deployed_drones.push_back(deployed);

    // Serialize
    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    // Deserialize into new world
    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "DroneBay deserialization succeeds");

    auto* e2 = world2.getEntity("drone_ship");
    assertTrue(e2 != nullptr, "Entity recreated");

    auto* bay2 = e2->getComponent<components::DroneBay>();
    assertTrue(bay2 != nullptr, "DroneBay component recreated");
    assertTrue(approxEqual(bay2->bay_capacity, 50.0f), "bay_capacity preserved");
    assertTrue(bay2->max_bandwidth == 50, "max_bandwidth preserved");
    assertTrue(bay2->stored_drones.size() == 1, "One stored drone");
    assertTrue(bay2->stored_drones[0].drone_id == "ogre", "Stored drone id preserved");
    assertTrue(approxEqual(bay2->stored_drones[0].damage, 55.0f), "Stored drone damage preserved");
    assertTrue(bay2->deployed_drones.size() == 1, "One deployed drone");
    assertTrue(bay2->deployed_drones[0].drone_id == "hobgoblin", "Deployed drone id preserved");
    assertTrue(approxEqual(bay2->deployed_drones[0].current_hp, 30.0f), "Deployed drone current_hp preserved");
}


void run_drone_system_tests() {
    testDroneLaunch();
    testDroneRecall();
    testDroneRecallAll();
    testDroneBandwidthLimit();
    testDroneCombatUpdate();
    testDroneDestroyedRemoval();
    testSerializeDeserializeDroneBay();
}
