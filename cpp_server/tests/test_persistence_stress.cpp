// Tests for: Phase 5: Persistence & Stress Testing
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "components/exploration_components.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "data/world_persistence.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Phase 5: Persistence & Stress Testing ====================

static void testPersistenceStress100Ships() {
    std::cout << "\n=== Persistence Stress: 100 Ships Save/Load ===" << std::endl;

    ecs::World world;

    // Create 100 ship entities with multiple components
    for (int i = 0; i < 100; ++i) {
        std::string id = "stress_persist_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);

        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 1000);
        pos->y = static_cast<float>((i % 10) * 200);
        pos->z = static_cast<float>((i / 10) * 300);

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 400.0f + static_cast<float>(i);
        hp->shield_max = 500.0f;
        hp->armor_hp = 250.0f + static_cast<float>(i % 50);
        hp->armor_max = 300.0f;
        hp->hull_hp = 150.0f + static_cast<float>(i % 30);
        hp->hull_max = 200.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = (i % 3 == 0) ? "Frigate" : ((i % 3 == 1) ? "Cruiser" : "Battleship");

        auto* ai = addComp<components::AI>(e);
        ai->state = components::AI::State::Idle;

        auto* lod = addComp<components::LODPriority>(e);
        lod->priority = (i < 5) ? 2.0f : 0.5f;
        lod->force_visible = (i < 5);
    }

    assertTrue(world.getEntityCount() == 100, "Created 100 entities for stress test");

    // Save to file
    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_stress_100ships.json";
    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "100-ship world saved to file");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "100-ship world loaded from file");
    assertTrue(world2.getEntityCount() == 100, "Loaded world has 100 entities");

    // Verify a sample of entities
    auto* e0 = world2.getEntity("stress_persist_ship_0");
    assertTrue(e0 != nullptr, "First ship entity found after load");
    auto* pos0 = e0->getComponent<components::Position>();
    assertTrue(pos0 != nullptr, "Position component on first ship");
    assertTrue(approxEqual(pos0->x, 0.0f), "First ship x position correct");

    auto* e50 = world2.getEntity("stress_persist_ship_50");
    assertTrue(e50 != nullptr, "Ship 50 entity found after load");
    auto* hp50 = e50->getComponent<components::Health>();
    assertTrue(hp50 != nullptr, "Health component on ship 50");
    assertTrue(approxEqual(hp50->shield_hp, 450.0f), "Ship 50 shield_hp preserved");

    auto* e99 = world2.getEntity("stress_persist_ship_99");
    assertTrue(e99 != nullptr, "Last ship entity found after load");
    auto* ship99 = e99->getComponent<components::Ship>();
    assertTrue(ship99 != nullptr, "Ship component on last entity");
    assertTrue(ship99->ship_type == "Frigate", "Last ship type preserved");

    // Verify LOD priorities survived
    auto* lod0 = e0->getComponent<components::LODPriority>();
    assertTrue(lod0 != nullptr, "LOD on first ship");
    assertTrue(approxEqual(lod0->priority, 2.0f), "High-priority LOD preserved");
    assertTrue(lod0->force_visible, "force_visible preserved on player ship");

    auto* lod50 = e50->getComponent<components::LODPriority>();
    assertTrue(lod50 != nullptr, "LOD on ship 50");
    assertTrue(approxEqual(lod50->priority, 0.5f), "Low-priority LOD preserved");

    // Clean up
    std::remove(filepath.c_str());
}

static void testPersistenceFleetStateFile() {
    std::cout << "\n=== Persistence: Fleet State File Save/Load ===" << std::endl;

    ecs::World world;

    // Create a fleet commander entity
    auto* fc = world.createEntity("fleet_fc");
    auto* fm_fc = addComp<components::FleetMembership>(fc);
    fm_fc->fleet_id = "fleet_alpha";
    fm_fc->role = "FleetCommander";
    fm_fc->squad_id = "squad_1";
    fm_fc->wing_id = "wing_1";
    fm_fc->active_bonuses["armor_hp_bonus"] = 0.10f;

    auto* ff_fc = addComp<components::FleetFormation>(fc);
    ff_fc->formation = components::FleetFormation::FormationType::Arrow;
    ff_fc->slot_index = 0;
    ff_fc->offset_x = 0.0f;
    ff_fc->offset_y = 0.0f;
    ff_fc->offset_z = 0.0f;

    auto* morale = addComp<components::FleetMorale>(fc);
    morale->morale_score = 42.0f;
    morale->wins = 8;
    morale->losses = 2;
    morale->ships_lost = 1;
    morale->morale_state = "Inspired";

    auto* cargo = addComp<components::FleetCargoPool>(fc);
    cargo->total_capacity = 50000;
    cargo->used_capacity = 12000;
    cargo->pooled_items["Stellium"] = 5000;
    cargo->pooled_items["Vanthium"] = 3000;
    cargo->contributor_ship_ids.push_back("fleet_fc");
    cargo->contributor_ship_ids.push_back("fleet_member_1");

    // Create a fleet member entity
    auto* member = world.createEntity("fleet_member_1");
    auto* fm_m = addComp<components::FleetMembership>(member);
    fm_m->fleet_id = "fleet_alpha";
    fm_m->role = "Member";
    fm_m->squad_id = "squad_1";
    fm_m->wing_id = "wing_1";

    auto* ff_m = addComp<components::FleetFormation>(member);
    ff_m->formation = components::FleetFormation::FormationType::Arrow;
    ff_m->slot_index = 1;
    ff_m->offset_x = -50.0f;
    ff_m->offset_y = 0.0f;
    ff_m->offset_z = 25.0f;
    ff_m->spacing_modifier = 0.8f;

    // Save to file
    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_fleet_state.json";
    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "Fleet state saved to file");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "Fleet state loaded from file");
    assertTrue(world2.getEntityCount() == 2, "Loaded 2 fleet entities");

    // Verify FC
    auto* fc2 = world2.getEntity("fleet_fc");
    assertTrue(fc2 != nullptr, "FC entity found after load");

    auto* fm_fc2 = fc2->getComponent<components::FleetMembership>();
    assertTrue(fm_fc2 != nullptr, "FleetMembership on FC after load");
    assertTrue(fm_fc2->fleet_id == "fleet_alpha", "FC fleet_id preserved");
    assertTrue(fm_fc2->role == "FleetCommander", "FC role preserved");

    auto* ff_fc2 = fc2->getComponent<components::FleetFormation>();
    assertTrue(ff_fc2 != nullptr, "FleetFormation on FC after load");
    assertTrue(ff_fc2->slot_index == 0, "FC slot_index preserved");

    auto* morale2 = fc2->getComponent<components::FleetMorale>();
    assertTrue(morale2 != nullptr, "FleetMorale on FC after load");
    assertTrue(approxEqual(morale2->morale_score, 42.0f), "morale_score preserved in file");
    assertTrue(morale2->wins == 8, "wins preserved in file");
    assertTrue(morale2->morale_state == "Inspired", "morale_state preserved in file");

    auto* cargo2 = fc2->getComponent<components::FleetCargoPool>();
    assertTrue(cargo2 != nullptr, "FleetCargoPool on FC after load");
    assertTrue(cargo2->total_capacity == 50000, "cargo total_capacity preserved");
    assertTrue(cargo2->used_capacity == 12000, "cargo used_capacity preserved");
    assertTrue(cargo2->pooled_items["Stellium"] == 5000, "cargo Stellium preserved");
    assertTrue(cargo2->pooled_items["Vanthium"] == 3000, "cargo Vanthium preserved");
    assertTrue(cargo2->contributor_ship_ids.size() == 2, "cargo contributors preserved");

    // Verify member
    auto* m2 = world2.getEntity("fleet_member_1");
    assertTrue(m2 != nullptr, "Member entity found after load");

    auto* fm_m2 = m2->getComponent<components::FleetMembership>();
    assertTrue(fm_m2 != nullptr, "FleetMembership on member after load");
    assertTrue(fm_m2->role == "Member", "Member role preserved");

    auto* ff_m2 = m2->getComponent<components::FleetFormation>();
    assertTrue(ff_m2 != nullptr, "FleetFormation on member after load");
    assertTrue(ff_m2->slot_index == 1, "Member slot_index preserved");
    assertTrue(approxEqual(ff_m2->offset_x, -50.0f), "Member offset_x preserved");
    assertTrue(approxEqual(ff_m2->spacing_modifier, 0.8f), "Member spacing_modifier preserved");

    // Clean up
    std::remove(filepath.c_str());
}

static void testPersistenceEconomyFile() {
    std::cout << "\n=== Persistence: Economy State File Save/Load ===" << std::endl;

    ecs::World world;

    // Create market hub with multiple orders
    auto* hub = world.createEntity("market_hub_1");
    auto* market = addComp<components::MarketHub>(hub);
    market->station_id = "station_jita";
    market->broker_fee_rate = 0.025;
    market->sales_tax_rate = 0.04;

    // Add sell order
    components::MarketHub::Order sell;
    sell.order_id = "sell_001";
    sell.item_id = "trit";
    sell.item_name = "Stellium";
    sell.owner_id = "npc_trader_1";
    sell.is_buy_order = false;
    sell.price_per_unit = 6.0;
    sell.quantity = 50000;
    sell.quantity_remaining = 45000;
    sell.duration_remaining = 172800.0f;
    sell.fulfilled = false;
    market->orders.push_back(sell);

    // Add buy order
    components::MarketHub::Order buy;
    buy.order_id = "buy_001";
    buy.item_id = "pye";
    buy.item_name = "Vanthium";
    buy.owner_id = "npc_trader_2";
    buy.is_buy_order = true;
    buy.price_per_unit = 12.0;
    buy.quantity = 20000;
    buy.quantity_remaining = 20000;
    buy.duration_remaining = 86400.0f;
    buy.fulfilled = false;
    market->orders.push_back(buy);

    // Create mineral deposits
    auto* belt = world.createEntity("asteroid_belt_1");
    auto* deposit = addComp<components::MineralDeposit>(belt);
    deposit->mineral_type = "Ferrite";
    deposit->quantity_remaining = 25000.0f;
    deposit->max_quantity = 50000.0f;
    deposit->yield_rate = 1.0f;
    deposit->volume_per_unit = 0.1f;

    // Create system resources
    auto* sys = world.createEntity("system_res_1");
    auto* sysres = addComp<components::SystemResources>(sys);
    components::SystemResources::ResourceEntry r1;
    r1.mineral_type = "Stellium";
    r1.total_quantity = 100000.0f;
    r1.remaining_quantity = 75000.0f;
    sysres->resources.push_back(r1);
    components::SystemResources::ResourceEntry r2;
    r2.mineral_type = "Cydrium";
    r2.total_quantity = 30000.0f;
    r2.remaining_quantity = 28000.0f;
    sysres->resources.push_back(r2);

    // Save to file
    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_economy_state.json";
    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "Economy state saved to file");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "Economy state loaded from file");
    assertTrue(world2.getEntityCount() == 3, "Loaded 3 economy entities");

    // Verify market hub
    auto* hub2 = world2.getEntity("market_hub_1");
    assertTrue(hub2 != nullptr, "Market hub found after load");
    auto* market2 = hub2->getComponent<components::MarketHub>();
    assertTrue(market2 != nullptr, "MarketHub component after load");
    assertTrue(market2->station_id == "station_jita", "station_id preserved");
    assertTrue(market2->orders.size() == 2, "Both orders preserved");
    assertTrue(market2->orders[0].order_id == "sell_001", "Sell order preserved");
    assertTrue(!market2->orders[0].is_buy_order, "Sell order type preserved");
    assertTrue(market2->orders[0].quantity_remaining == 45000, "Sell order qty remaining preserved");
    assertTrue(market2->orders[1].order_id == "buy_001", "Buy order preserved");
    assertTrue(market2->orders[1].is_buy_order, "Buy order type preserved");

    // Verify mineral deposit
    auto* belt2 = world2.getEntity("asteroid_belt_1");
    assertTrue(belt2 != nullptr, "Asteroid belt found after load");
    auto* deposit2 = belt2->getComponent<components::MineralDeposit>();
    assertTrue(deposit2 != nullptr, "MineralDeposit after load");
    assertTrue(deposit2->mineral_type == "Ferrite", "mineral_type preserved");
    assertTrue(approxEqual(deposit2->quantity_remaining, 25000.0f), "deposit qty remaining preserved");

    // Verify system resources
    auto* sys2 = world2.getEntity("system_res_1");
    assertTrue(sys2 != nullptr, "System resources found after load");
    auto* sysres2 = sys2->getComponent<components::SystemResources>();
    assertTrue(sysres2 != nullptr, "SystemResources after load");
    assertTrue(sysres2->resources.size() == 2, "Resource entries preserved");
    assertTrue(sysres2->resources[0].mineral_type == "Stellium", "Stellium entry preserved");
    assertTrue(approxEqual(sysres2->resources[0].remaining_quantity, 75000.0f), "Stellium remaining preserved");

    // Clean up
    std::remove(filepath.c_str());
}


void run_persistence_stress_tests() {
    testPersistenceStress100Ships();
    testPersistenceFleetStateFile();
    testPersistenceEconomyFile();
}
