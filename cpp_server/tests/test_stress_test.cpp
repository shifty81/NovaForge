// Tests for: 100+ Ship Fleet Stress Test, Phase 5 Continued: 200-Ship Multi-System Stress Test
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/lod_system.h"
#include "data/world_persistence.h"
#include "systems/movement_system.h"
#include "systems/shield_recharge_system.h"
#include "systems/spatial_hash_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== 100+ Ship Fleet Stress Test ====================

static void testFleetStress150Ships() {
    std::cout << "\n=== Fleet Stress Test: 150 Ships ===" << std::endl;
    ecs::World world;

    // Create 150 ship entities with Position, Health, Ship, and AI components
    std::vector<ecs::Entity*> ships;
    for (int i = 0; i < 150; ++i) {
        std::string id = "stress_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);
        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 100);
        pos->y = 0.0f;
        pos->z = static_cast<float>((i % 10) * 50);

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 500.0f;
        hp->shield_max = 500.0f;
        hp->armor_hp = 300.0f;
        hp->armor_max = 300.0f;
        hp->hull_hp = 200.0f;
        hp->hull_max = 200.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = "Frigate";

        auto* ai = addComp<components::AI>(e);
        ai->state = components::AI::State::Idle;

        auto* lod = addComp<components::LODPriority>(e);
        lod->priority = (i < 10) ? 2.0f : 0.5f;  // first 10 ships high priority

        ships.push_back(e);
    }

    assertTrue(ships.size() == 150, "Created 150 ship entities");

    // Verify all entities were created and have components
    int valid = 0;
    for (auto* e : ships) {
        if (e->getComponent<components::Position>() &&
            e->getComponent<components::Health>() &&
            e->getComponent<components::Ship>() &&
            e->getComponent<components::AI>() &&
            e->getComponent<components::LODPriority>()) {
            ++valid;
        }
    }
    assertTrue(valid == 150, "All 150 ships have all required components");

    // Count high-priority vs low-priority
    int highPriority = 0;
    int lowPriority = 0;
    for (auto* e : ships) {
        auto* lod = e->getComponent<components::LODPriority>();
        if (lod->priority >= 1.5f) ++highPriority;
        else ++lowPriority;
    }
    assertTrue(highPriority == 10, "10 ships are high priority");
    assertTrue(lowPriority == 140, "140 ships are low priority");
}

static void testFleetStressSystemUpdates() {
    std::cout << "\n=== Fleet Stress Test: System Updates ===" << std::endl;
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);

    // Create 100 ships with Health components for shield recharge
    for (int i = 0; i < 100; ++i) {
        std::string id = "stress_shield_" + std::to_string(i);
        auto* e = world.createEntity(id);
        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 250.0f;
        hp->shield_max = 500.0f;
        hp->shield_recharge_rate = 10.0f;
    }

    // Run 10 update ticks
    for (int tick = 0; tick < 10; ++tick) {
        shieldSys.update(1.0f);
    }

    // Verify shields recharged across all entities
    int recharged = 0;
    auto entities = world.getEntities();
    for (auto* e : entities) {
        auto* hp = e->getComponent<components::Health>();
        if (hp && hp->shield_hp > 250.0f) {
            ++recharged;
        }
    }
    assertTrue(recharged == 100, "All 100 ships recharged shields");
}


// ==================== Phase 5 Continued: 200-Ship Multi-System Stress Test ====================

static void testStress200ShipMultiSystem() {
    std::cout << "\n=== Stress Test: 200 Ships Multi-System Tick ===" << std::endl;
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);
    systems::MovementSystem moveSys(&world);
    systems::LODSystem lodSys(&world);
    systems::SpatialHashSystem spatialSys(&world);
    spatialSys.setCellSize(5000.0f);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    // Create 200 ships spread across space
    for (int i = 0; i < 200; ++i) {
        std::string id = "multi_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);

        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>((i % 20) * 3000);
        pos->y = static_cast<float>((i / 20) * 3000);
        pos->z = 0.0f;

        auto* vel = addComp<components::Velocity>(e);
        vel->vx = 10.0f;
        vel->vy = 0.0f;
        vel->vz = 0.0f;

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 250.0f;
        hp->shield_max = 500.0f;
        hp->shield_recharge_rate = 5.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = (i % 3 == 0) ? "Frigate" : ((i % 3 == 1) ? "Cruiser" : "Battleship");
        vel->max_speed = 300.0f;

        auto* lod = addComp<components::LODPriority>(e);
        lod->priority = 1.0f;
        lod->force_visible = (i == 0);  // only player ship forced
    }

    assertTrue(world.getEntityCount() == 200, "Created 200 ship entities");

    // Run 10 simulation ticks with all systems
    for (int tick = 0; tick < 10; ++tick) {
        float dt = 0.1f;  // 100ms tick (10 Hz)
        moveSys.update(dt);
        shieldSys.update(dt);
        lodSys.update(dt);
        spatialSys.update(dt);
    }

    // Verify shields recharged
    int recharged = 0;
    auto entities = world.getEntities();
    for (auto* e : entities) {
        auto* hp = e->getComponent<components::Health>();
        if (hp && hp->shield_hp > 250.0f) ++recharged;
    }
    assertTrue(recharged == 200, "All 200 ships recharged shields across 10 ticks");

    // Verify LOD computed (player ship should be full detail)
    auto* playerLod = world.getEntity("multi_ship_0")->getComponent<components::LODPriority>();
    assertTrue(approxEqual(playerLod->priority, 2.0f), "Player ship at full detail (force_visible)");

    // Verify LOD counts sum to 200
    int lodTotal = lodSys.getFullDetailCount() + lodSys.getReducedCount() +
                   lodSys.getMergedCount() + lodSys.getImpostorCount();
    assertTrue(lodTotal == 200, "LOD tier counts sum to 200");

    // Verify spatial hash indexed all 200 entities
    assertTrue(spatialSys.getIndexedEntityCount() == 200, "Spatial hash indexed 200 entities");

    // Verify spatial query returns reasonable neighbourhood
    auto nearby = spatialSys.queryNear(0.0f, 0.0f, 0.0f, 5000.0f);
    assertTrue(!nearby.empty(), "Spatial query finds nearby ships");
    assertTrue(nearby.size() <= 200, "Spatial query doesn't exceed total entities");

    // Verify entities moved (velocity applied over 10 ticks)
    auto* ship50 = world.getEntity("multi_ship_50");
    auto* pos50 = ship50->getComponent<components::Position>();
    // Original x was (50%20)*3000 = 30000.0, moved +10.0 * 10 ticks * 0.1s = +10.0m
    assertTrue(pos50->x > 30000.0f, "Ship 50 moved forward from velocity");
}

static void testStress200ShipPersistence() {
    std::cout << "\n=== Stress Test: 200 Ships Save/Load ===" << std::endl;
    ecs::World world;

    for (int i = 0; i < 200; ++i) {
        std::string id = "persist200_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);

        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 500);
        pos->y = static_cast<float>((i % 20) * 100);
        pos->z = 0.0f;

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 400.0f + static_cast<float>(i % 100);
        hp->shield_max = 500.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = "Frigate";

        auto* lod = addComp<components::LODPriority>(e);
        lod->priority = (i < 10) ? 2.0f : 0.5f;
    }

    assertTrue(world.getEntityCount() == 200, "Created 200 entities");

    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_stress_200ships.json.gz";

    bool saved = persistence.saveWorldCompressed(&world, filepath);
    assertTrue(saved, "200-ship compressed save succeeded");

    ecs::World world2;
    bool loaded = persistence.loadWorldCompressed(&world2, filepath);
    assertTrue(loaded, "200-ship compressed load succeeded");
    assertTrue(world2.getEntityCount() == 200, "Loaded world has 200 entities");

    // Verify sample entities
    auto* first = world2.getEntity("persist200_ship_0");
    assertTrue(first != nullptr, "First ship exists after load");
    auto* last = world2.getEntity("persist200_ship_199");
    assertTrue(last != nullptr, "Last ship exists after load");

    auto* hp199 = last->getComponent<components::Health>();
    assertTrue(hp199 != nullptr, "Health preserved on last ship");
    assertTrue(approxEqual(hp199->shield_hp, 499.0f), "Shield HP preserved (400 + 199%100 = 499)");

    std::remove(filepath.c_str());
}


void run_stress_test_tests() {
    testFleetStress150Ships();
    testFleetStressSystemUpdates();
    testStress200ShipMultiSystem();
    testStress200ShipPersistence();
}
