// Tests for: Mining System Tests, Mining Drone Tests, AI Mining State Test, AI Profit-Based Deposit Selection Tests, AI Hauling Behavior Tests, AI Full Economic Cycle Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "components/npc_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ai_system.h"
#include "pcg/salvage_system.h"
#include "systems/drone_system.h"
#include "systems/mining_system.h"
#include "systems/wreck_salvage_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mining System Tests ====================

static void testMiningCreateDeposit() {
    std::cout << "\n=== Mining: Create Deposit ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string id = mineSys.createDeposit("Ferrite", 5000.0f, 100.0f, 0.0f, 0.0f);
    assertTrue(!id.empty(), "Deposit entity created");

    auto* entity = world.getEntity(id);
    assertTrue(entity != nullptr, "Deposit entity exists in world");

    auto* dep = entity->getComponent<components::MineralDeposit>();
    assertTrue(dep != nullptr, "Deposit has MineralDeposit component");
    assertTrue(dep->mineral_type == "Ferrite", "Mineral type is Ferrite");
    assertTrue(approxEqual(dep->quantity_remaining, 5000.0f), "Quantity remaining is 5000");
    assertTrue(!dep->isDepleted(), "Deposit is not depleted");

    auto* pos = entity->getComponent<components::Position>();
    assertTrue(pos != nullptr, "Deposit has Position component");
    assertTrue(approxEqual(pos->x, 100.0f), "Deposit x position correct");
}

static void testMiningStartStop() {
    std::cout << "\n=== Mining: Start and Stop ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Galvite", 1000.0f, 0.0f, 0.0f, 0.0f);

    auto* miner = world.createEntity("miner_1");
    auto* pos = addComp<components::Position>(miner);
    pos->x = 5000.0f; // within default 10000m range
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 50.0f;
    laser->cycle_time = 10.0f;
    addComp<components::Inventory>(miner);

    bool started = mineSys.startMining("miner_1", dep_id);
    assertTrue(started, "Mining started successfully");
    assertTrue(laser->active, "Laser is active");
    assertTrue(mineSys.getActiveMinerCount() == 1, "One active miner");

    bool stopped = mineSys.stopMining("miner_1");
    assertTrue(stopped, "Mining stopped successfully");
    assertTrue(!laser->active, "Laser is inactive after stop");
    assertTrue(mineSys.getActiveMinerCount() == 0, "No active miners");
}

static void testMiningRangeCheck() {
    std::cout << "\n=== Mining: Range Check ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Ferrite", 1000.0f, 0.0f, 0.0f, 0.0f);

    auto* miner = world.createEntity("miner_far");
    auto* pos = addComp<components::Position>(miner);
    pos->x = 20000.0f; // too far (default range 10000m)
    addComp<components::MiningLaser>(miner);
    addComp<components::Inventory>(miner);

    bool started = mineSys.startMining("miner_far", dep_id);
    assertTrue(!started, "Mining fails when out of range");
}

static void testMiningCycleCompletion() {
    std::cout << "\n=== Mining: Cycle Completion ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Ferrite", 1000.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    auto* miner = world.createEntity("miner_1");
    auto* pos = addComp<components::Position>(miner);
    pos->x = 100.0f;
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 50.0f;
    laser->cycle_time = 10.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 500.0f;

    mineSys.startMining("miner_1", dep_id);

    // Advance 10 seconds — one full cycle
    mineSys.update(10.0f);

    assertTrue(inv->items.size() == 1, "Ore item added to inventory");
    assertTrue(inv->items[0].item_id == "Ferrite", "Mined Ferrite");
    assertTrue(inv->items[0].quantity == 50, "Mined 50 units");

    auto* dep = world.getEntity(dep_id)->getComponent<components::MineralDeposit>();
    assertTrue(approxEqual(dep->quantity_remaining, 950.0f), "Deposit reduced by 50");
}

static void testMiningDepletedDeposit() {
    std::cout << "\n=== Mining: Depleted Deposit ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    // Small deposit — only 20 units
    std::string dep_id = mineSys.createDeposit("Heliore", 20.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    auto* miner = world.createEntity("miner_1");
    addComp<components::Position>(miner);
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 50.0f;
    laser->cycle_time = 5.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 500.0f;

    mineSys.startMining("miner_1", dep_id);
    mineSys.update(5.0f);

    // Should only get 20 units (deposit was 20, yield was 50)
    assertTrue(inv->items.size() == 1, "Ore item added");
    assertTrue(inv->items[0].quantity == 20, "Only mined available 20 units");

    auto* dep = world.getEntity(dep_id)->getComponent<components::MineralDeposit>();
    assertTrue(dep->isDepleted(), "Deposit is depleted");
    assertTrue(!laser->active, "Laser stops when deposit depleted");
}

static void testMiningCargoFull() {
    std::cout << "\n=== Mining: Cargo Full ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Ferrite", 10000.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    auto* miner = world.createEntity("miner_1");
    addComp<components::Position>(miner);
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 100.0f;
    laser->cycle_time = 5.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 50.0f; // only 50 m3 free, vol_per_unit=1.0

    mineSys.startMining("miner_1", dep_id);
    mineSys.update(5.0f);

    // Should only mine 50 units (cargo limit)
    assertTrue(inv->items.size() == 1, "Ore item added");
    assertTrue(inv->items[0].quantity == 50, "Capped by cargo capacity");
}

static void testMiningOreStacking() {
    std::cout << "\n=== Mining: Ore Stacking ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Ferrite", 10000.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    auto* miner = world.createEntity("miner_1");
    addComp<components::Position>(miner);
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 30.0f;
    laser->cycle_time = 5.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 5000.0f;

    mineSys.startMining("miner_1", dep_id);

    // Complete two full cycles
    mineSys.update(5.0f);
    mineSys.update(5.0f);

    assertTrue(inv->items.size() == 1, "Ore stacked into single item");
    assertTrue(inv->items[0].quantity == 60, "Two cycles stacked: 30+30=60");
}


// ==================== Mining Drone Tests ====================

static void testMiningDroneLaunchAndMine() {
    std::cout << "\n=== Mining Drone: Launch and Mine ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::MiningSystem mineSys(&world);

    // Create a mineral deposit
    std::string dep_id = mineSys.createDeposit("Ferrite", 5000.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    // Create a ship with mining drones
    auto* ship = world.createEntity("ship_1");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 5000.0f;
    addComp<components::Position>(ship);

    components::DroneBay::DroneInfo mDrone;
    mDrone.drone_id = "mining_drone_1";
    mDrone.name = "Mining Drone I";
    mDrone.type = "mining_drone";
    mDrone.mining_yield = 25.0f;
    mDrone.rate_of_fire = 60.0f; // 60s cycle
    mDrone.hitpoints = 50.0f;
    mDrone.current_hp = 50.0f;
    mDrone.bandwidth_use = 5;
    mDrone.volume = 5.0f;
    bay->stored_drones.push_back(mDrone);

    // Launch drone and set mining target
    bool launched = droneSys.launchDrone("ship_1", "mining_drone_1");
    assertTrue(launched, "Mining drone launched");
    assertTrue(droneSys.getDeployedCount("ship_1") == 1, "One drone deployed");

    bool targeted = droneSys.setMiningTarget("ship_1", dep_id);
    assertTrue(targeted, "Mining target set");

    // Complete one cycle (drone rate_of_fire = 60s, but cooldown starts at 0)
    droneSys.update(0.0f); // first tick: mines immediately (cooldown=0)

    assertTrue(inv->items.size() == 1, "Ore mined by drone");
    assertTrue(inv->items[0].item_id == "Ferrite", "Mined correct mineral");
    assertTrue(inv->items[0].quantity == 25, "Mined correct amount");

    auto* dep = world.getEntity(dep_id)->getComponent<components::MineralDeposit>();
    assertTrue(approxEqual(dep->quantity_remaining, 4975.0f), "Deposit reduced by mining drone");
}

static void testSalvageDroneLaunchAndSalvage() {
    std::cout << "\n=== Salvage Drone: Launch and Salvage ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    // Create a wreck with loot
    std::string wreck_id = wreckSys.createWreck("dead_npc", 100.0f, 0.0f, 0.0f);
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck_inv = wreck_entity->getComponent<components::Inventory>();
    components::Inventory::Item salvage;
    salvage.item_id = "salvage_metal";
    salvage.name = "Salvaged Metal";
    salvage.type = "salvage";
    salvage.quantity = 5;
    salvage.volume = 0.5f;
    wreck_inv->items.push_back(salvage);

    // Create ship with salvage drone
    auto* ship = world.createEntity("ship_1");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 500.0f;
    addComp<components::Position>(ship);

    components::DroneBay::DroneInfo sDrone;
    sDrone.drone_id = "salvage_drone_1";
    sDrone.name = "Salvage Drone I";
    sDrone.type = "salvage_drone";
    sDrone.salvage_chance = 1.0f; // 100% for testing
    sDrone.rate_of_fire = 10.0f;
    sDrone.hitpoints = 30.0f;
    sDrone.current_hp = 30.0f;
    sDrone.bandwidth_use = 5;
    sDrone.volume = 5.0f;
    bay->stored_drones.push_back(sDrone);

    bool launched = droneSys.launchDrone("ship_1", "salvage_drone_1");
    assertTrue(launched, "Salvage drone launched");

    bool targeted = droneSys.setSalvageTarget("ship_1", wreck_id);
    assertTrue(targeted, "Salvage target set");

    // First tick: salvage immediately (cooldown=0), chance=1.0 guaranteed
    droneSys.update(0.0f);

    // Wreck should now be salvaged
    auto* wreck = wreck_entity->getComponent<components::Wreck>();
    assertTrue(wreck->salvaged, "Wreck marked as salvaged");

    // Items transferred to ship
    assertTrue(inv->items.size() == 1, "Salvage transferred to ship");
    assertTrue(inv->items[0].item_id == "salvage_metal", "Correct salvage item");
    assertTrue(inv->items[0].quantity == 5, "Correct salvage quantity");
}

static void testSalvageDroneAlreadySalvaged() {
    std::cout << "\n=== Salvage Drone: Already Salvaged ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("dead_npc2", 0.0f, 0.0f, 0.0f);
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck = wreck_entity->getComponent<components::Wreck>();
    wreck->salvaged = true; // already salvaged

    auto* ship = world.createEntity("ship_2");
    auto* bay = addComp<components::DroneBay>(ship);
    addComp<components::Inventory>(ship);
    addComp<components::Position>(ship);

    // Cannot set salvage target on already-salvaged wreck
    bool targeted = droneSys.setSalvageTarget("ship_2", wreck_id);
    assertTrue(!targeted, "Cannot target already-salvaged wreck");
}

static void testMiningDroneTargetDepletedDeposit() {
    std::cout << "\n=== Mining Drone: Depleted Deposit ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Ferrite", 0.0f, 0.0f, 0.0f, 0.0f);

    auto* ship = world.createEntity("ship_3");
    auto* bay = addComp<components::DroneBay>(ship);
    addComp<components::Inventory>(ship);
    addComp<components::Position>(ship);

    bool targeted = droneSys.setMiningTarget("ship_3", dep_id);
    assertTrue(!targeted, "Cannot target depleted deposit");
}


// ==================== AI Mining State Test ====================

static void testAIMiningState() {
    std::cout << "\n=== AI: Mining State ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("miner_npc");
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;

    assertTrue(ai->state == components::AI::State::Mining, "AI state set to Mining");
    // Mining state is distinct from other states
    assertTrue(ai->state != components::AI::State::Idle, "Mining != Idle");
    assertTrue(ai->state != components::AI::State::Attacking, "Mining != Attacking");
}

static void testAIMiningBehaviorActivatesLaser() {
    std::cout << "\n=== AI: Mining Behavior Activates Laser ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);
    systems::MiningSystem miningSys(&world);

    // Create a mineral deposit nearby
    auto* deposit = world.createEntity("deposit1");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    // Create an AI miner with MiningLaser
    auto* npc = world.createEntity("miner_npc");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "deposit1";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 1000.0f;

    // Run AI update — mining behavior should activate the laser
    aiSys.update(1.0f);

    assertTrue(laser->active, "Mining behavior activates laser");
    assertTrue(laser->target_deposit_id == "deposit1", "Laser targets the deposit");
}

static void testAIMiningIdleFindsDeposit() {
    std::cout << "\n=== AI: Idle Miner Finds Deposit ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a mineral deposit within awareness range
    auto* deposit = world.createEntity("deposit_nearby");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 5000.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Galvite";
    dep->quantity_remaining = 3000.0f;

    // Create a passive AI miner at origin, idle
    auto* npc = world.createEntity("miner2");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Idle;
    ai->awareness_range = 50000.0f;
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;

    // Idle behavior should find the deposit and start approaching
    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Approaching, "Idle miner transitions to Approaching");
    assertTrue(ai->target_entity_id == "deposit_nearby", "Miner targets the deposit");
}

static void testAIMiningApproachTransitions() {
    std::cout << "\n=== AI: Miner Approach Transitions to Mining ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create deposit very close
    auto* deposit = world.createEntity("dep_close");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 1000.0f;

    // Create NPC approaching the deposit (within mining range)
    auto* npc = world.createEntity("miner3");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Approaching;
    ai->target_entity_id = "dep_close";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;

    // Approach should transition to Mining when within range
    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Mining, "Approach transitions to Mining near deposit");
}

static void testAIMiningStopsOnDepletedDeposit() {
    std::cout << "\n=== AI: Mining Stops When Deposit Depleted ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create an empty deposit
    auto* deposit = world.createEntity("empty_dep");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 0.0f;

    auto* npc = world.createEntity("miner4");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 100.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "empty_dep";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = true;
    laser->target_deposit_id = "empty_dep";

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Idle, "Mining stops on depleted deposit");
    assertTrue(ai->target_entity_id.empty(), "Target cleared on depleted deposit");
}

static void testAIMiningStopsOnCargoFull() {
    std::cout << "\n=== AI: Mining Stops When Cargo Full ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* deposit = world.createEntity("dep_full");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("miner5");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 100.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "dep_full";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = true;
    // Fill cargo completely
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 100.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 1000;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Idle, "Mining stops on cargo full");
}

static void testAIFindNearestDeposit() {
    std::cout << "\n=== AI: Find Nearest Deposit ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create two deposits at different distances
    auto* far_dep = world.createEntity("far_dep");
    auto* fdpos = addComp<components::Position>(far_dep);
    fdpos->x = 30000.0f; fdpos->y = 0.0f; fdpos->z = 0.0f;
    auto* fd = addComp<components::MineralDeposit>(far_dep);
    fd->mineral_type = "Galvite";
    fd->quantity_remaining = 5000.0f;

    auto* near_dep = world.createEntity("near_dep");
    auto* ndpos = addComp<components::Position>(near_dep);
    ndpos->x = 5000.0f; ndpos->y = 0.0f; ndpos->z = 0.0f;
    auto* nd = addComp<components::MineralDeposit>(near_dep);
    nd->mineral_type = "Ferrite";
    nd->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("searcher");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->awareness_range = 50000.0f;

    auto* found = aiSys.findNearestDeposit(npc);
    assertTrue(found != nullptr, "Found a deposit");
    assertTrue(found->getId() == "near_dep", "Found nearest deposit");
}


// ==================== AI Profit-Based Deposit Selection Tests ====================

static void testAIFindMostProfitableDepositWithPrices() {
    std::cout << "\n=== AI: Find Most Profitable Deposit With Market Prices ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a SupplyDemand entity with market prices
    auto* market = world.createEntity("system_market");
    auto* sd = addComp<components::SupplyDemand>(market);
    sd->addCommodity("Ferrite", 10.0f, 100.0f, 100.0f);   // cheap
    sd->addCommodity("Galvite", 500.0f, 100.0f, 100.0f);  // expensive

    // Cheap deposit close by
    auto* close_dep = world.createEntity("close_dep");
    auto* cdpos = addComp<components::Position>(close_dep);
    cdpos->x = 1000.0f; cdpos->y = 0.0f; cdpos->z = 0.0f;
    auto* cd = addComp<components::MineralDeposit>(close_dep);
    cd->mineral_type = "Ferrite";
    cd->quantity_remaining = 5000.0f;

    // Expensive deposit farther away (but high value/distance)
    auto* far_dep = world.createEntity("far_dep");
    auto* fdpos = addComp<components::Position>(far_dep);
    fdpos->x = 10000.0f; fdpos->y = 0.0f; fdpos->z = 0.0f;
    auto* fd = addComp<components::MineralDeposit>(far_dep);
    fd->mineral_type = "Galvite";
    fd->quantity_remaining = 5000.0f;

    // Create NPC at origin
    auto* npc = world.createEntity("profit_miner");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->awareness_range = 50000.0f;

    // Ferrite: 10/1000 = 0.01 score
    // Galvite: 500/10000 = 0.05 score — should be selected
    auto* best = aiSys.findMostProfitableDeposit(npc);
    assertTrue(best != nullptr, "Found a profitable deposit");
    assertTrue(best->getId() == "far_dep", "Selected higher-value Galvite deposit");
}

static void testAIFindMostProfitableDepositNoMarket() {
    std::cout << "\n=== AI: Find Most Profitable Deposit Without Market Data ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // No SupplyDemand entity — should fall back to nearest
    auto* far = world.createEntity("far");
    auto* fpos = addComp<components::Position>(far);
    fpos->x = 20000.0f;
    auto* fd = addComp<components::MineralDeposit>(far);
    fd->quantity_remaining = 5000.0f;

    auto* near = world.createEntity("near");
    auto* npos = addComp<components::Position>(near);
    npos->x = 3000.0f;
    auto* nd = addComp<components::MineralDeposit>(near);
    nd->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("miner");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(npc);
    auto* ai = addComp<components::AI>(npc);
    ai->awareness_range = 50000.0f;

    auto* best = aiSys.findMostProfitableDeposit(npc);
    assertTrue(best != nullptr, "Found a deposit (fallback to nearest)");
    assertTrue(best->getId() == "near", "Falls back to nearest without market data");
}

static void testAIFindMostProfitableDepositPrefersCloseWhenEqual() {
    std::cout << "\n=== AI: Profitable Deposit Prefers Close When Price Equal ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* market = world.createEntity("sys");
    auto* sd = addComp<components::SupplyDemand>(market);
    sd->addCommodity("Ferrite", 100.0f, 100.0f, 100.0f);

    // Two deposits with same mineral at different distances
    auto* close = world.createEntity("close");
    auto* cpos = addComp<components::Position>(close);
    cpos->x = 2000.0f;
    auto* cd = addComp<components::MineralDeposit>(close);
    cd->mineral_type = "Ferrite";
    cd->quantity_remaining = 5000.0f;

    auto* far = world.createEntity("far");
    auto* fpos = addComp<components::Position>(far);
    fpos->x = 8000.0f;
    auto* fd = addComp<components::MineralDeposit>(far);
    fd->mineral_type = "Ferrite";
    fd->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("miner");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);
    auto* ai = addComp<components::AI>(npc);
    ai->awareness_range = 50000.0f;

    auto* best = aiSys.findMostProfitableDeposit(npc);
    assertTrue(best != nullptr, "Found deposit");
    assertTrue(best->getId() == "close", "Prefers closer deposit when price is equal");
}


// ==================== AI Hauling Behavior Tests ====================

static void testAIHaulingState() {
    std::cout << "\n=== AI: Hauling State ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("hauler_npc");
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Hauling;

    assertTrue(ai->state == components::AI::State::Hauling, "AI state set to Hauling");
    assertTrue(ai->state != components::AI::State::Mining, "Hauling != Mining");
    assertTrue(ai->state != components::AI::State::Idle, "Hauling != Idle");
}

static void testAIHaulingMovesToStation() {
    std::cout << "\n=== AI: Hauling Moves Toward Station ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a station far away
    auto* station = world.createEntity("station1");
    auto* spos = addComp<components::Position>(station);
    spos->x = 10000.0f; spos->y = 0.0f; spos->z = 0.0f;

    // Create NPC hauling toward station
    auto* npc = world.createEntity("hauler");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 200.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Hauling;
    ai->haul_station_id = "station1";

    aiSys.update(1.0f);

    // Should be moving toward station (positive vx)
    assertTrue(vel->vx > 0.0f, "Hauler moves toward station (vx > 0)");
    assertTrue(ai->state == components::AI::State::Hauling, "Still hauling while far away");
}

static void testAIHaulingSellsCargoAtStation() {
    std::cout << "\n=== AI: Hauling Sells Cargo At Station ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create market with prices
    auto* market = world.createEntity("market_sys");
    auto* sd = addComp<components::SupplyDemand>(market);
    sd->addCommodity("Ferrite", 50.0f, 100.0f, 100.0f);

    // Create station within docking range
    auto* station = world.createEntity("station1");
    auto* spos = addComp<components::Position>(station);
    spos->x = 100.0f; spos->y = 0.0f; spos->z = 0.0f;

    // Create NPC near station with cargo
    auto* npc = world.createEntity("hauler");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Hauling;
    ai->haul_station_id = "station1";
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 1000.0f;
    // Add 100 units of Ferrite to cargo
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 100;
    ore.volume = 0.1f;
    inv->items.push_back(ore);
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->wallet = 1000.0;

    aiSys.update(1.0f);

    // Should have sold cargo: 100 * 50 = 5000 Credits earned
    assertTrue(inv->items.empty(), "Cargo sold (inventory cleared)");
    assertTrue(intent->wallet == 6000.0, "Wallet increased by 5000 (100 * 50)");
    assertTrue(ai->state == components::AI::State::Idle, "Returns to Idle after selling");
}

static void testAIHaulingNoStationGoesIdle() {
    std::cout << "\n=== AI: Hauling Without Station Goes Idle ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("hauler");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Hauling;
    ai->haul_station_id = "";  // no station

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Idle, "Goes idle without station");
}

static void testAIMiningCargoFullTransitionsToHauling() {
    std::cout << "\n=== AI: Mining Cargo Full Transitions to Hauling ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create deposit
    auto* deposit = world.createEntity("deposit1");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    // Create miner with full cargo and haul station set
    auto* npc = world.createEntity("miner");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "deposit1";
    ai->haul_station_id = "station1";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = true;
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 10.0f;  // tiny hold
    // Fill cargo completely
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 200;
    ore.volume = 0.1f;
    inv->items.push_back(ore);  // 200 * 0.1 = 20.0 > 10.0 max

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Hauling, "Cargo full transitions to Hauling");
    assertTrue(!laser->active, "Mining laser deactivated");
}

static void testAIMiningCargoFullNoStationGoesIdle() {
    std::cout << "\n=== AI: Mining Cargo Full Without Station Goes Idle ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* deposit = world.createEntity("deposit1");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("miner");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "deposit1";
    ai->haul_station_id = "";  // no station
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = true;
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 10.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 200;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Idle, "Cargo full goes Idle without station");
}


// ==================== AI Full Economic Cycle Tests ====================

static void testAIFullMineHaulSellCycle() {
    std::cout << "\n=== AI: Full Mine-Haul-Sell Cycle ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Market prices
    auto* market = world.createEntity("market");
    auto* sd = addComp<components::SupplyDemand>(market);
    sd->addCommodity("Ferrite", 25.0f, 100.0f, 100.0f);

    // Station for selling
    auto* station = world.createEntity("sell_station");
    auto* spos = addComp<components::Position>(station);
    spos->x = 200.0f;

    // Deposit
    auto* deposit = world.createEntity("ore_deposit");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    // NPC miner starting idle
    auto* npc = world.createEntity("full_cycle_miner");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Idle;
    ai->awareness_range = 50000.0f;
    ai->haul_station_id = "sell_station";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 10.0f;  // small hold
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->wallet = 0.0;

    // Step 1: Idle → should find deposit and start approaching
    aiSys.update(1.0f);
    assertTrue(ai->state == components::AI::State::Approaching, "Step 1: Idle -> Approaching");
    assertTrue(ai->target_entity_id == "ore_deposit", "Step 1: Target is deposit");

    // Step 2: Simulate arrival at deposit (move NPC close)
    pos->x = 100.0f;
    ai->state = components::AI::State::Mining;
    aiSys.update(1.0f);
    assertTrue(laser->active, "Step 2: Laser activated while mining");

    // Step 3: Fill cargo, trigger hauling
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 200;
    ore.volume = 0.1f;
    inv->items.push_back(ore);  // overfill
    aiSys.update(1.0f);
    assertTrue(ai->state == components::AI::State::Hauling, "Step 3: Mining -> Hauling (cargo full)");

    // Step 4: Move NPC to station and sell
    pos->x = 200.0f;
    aiSys.update(1.0f);
    assertTrue(inv->items.empty(), "Step 4: Cargo sold");
    assertTrue(intent->wallet == 5000.0, "Step 4: Earned 200 * 25 = 5000 Credits");
    assertTrue(ai->state == components::AI::State::Idle, "Step 4: Back to Idle");
}


void run_mining_system_tests() {
    testMiningCreateDeposit();
    testMiningStartStop();
    testMiningRangeCheck();
    testMiningCycleCompletion();
    testMiningDepletedDeposit();
    testMiningCargoFull();
    testMiningOreStacking();
    testMiningDroneLaunchAndMine();
    testSalvageDroneLaunchAndSalvage();
    testSalvageDroneAlreadySalvaged();
    testMiningDroneTargetDepletedDeposit();
    testAIMiningState();
    testAIMiningBehaviorActivatesLaser();
    testAIMiningIdleFindsDeposit();
    testAIMiningApproachTransitions();
    testAIMiningStopsOnDepletedDeposit();
    testAIMiningStopsOnCargoFull();
    testAIFindNearestDeposit();
    testAIFindMostProfitableDepositWithPrices();
    testAIFindMostProfitableDepositNoMarket();
    testAIFindMostProfitableDepositPrefersCloseWhenEqual();
    testAIHaulingState();
    testAIHaulingMovesToStation();
    testAIHaulingSellsCargoAtStation();
    testAIHaulingNoStationGoesIdle();
    testAIMiningCargoFullTransitionsToHauling();
    testAIMiningCargoFullNoStationGoesIdle();
    testAIFullMineHaulSellCycle();
}
