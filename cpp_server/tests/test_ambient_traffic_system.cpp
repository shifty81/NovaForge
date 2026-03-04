// Tests for: Ambient Traffic System Tests
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/ambient_traffic_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Ambient Traffic System Tests ====================

static void testAmbientTrafficDefaults() {
    std::cout << "\n=== Ambient Traffic: Defaults ===" << std::endl;
    ecs::World world;

    auto* sys = world.createEntity("traffic_sys");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);

    assertTrue(traffic->spawn_timer == 60.0f, "Default spawn timer is 60s");
    assertTrue(traffic->active_traffic_count == 0, "No active traffic initially");
    assertTrue(traffic->pending_spawns.empty(), "No pending spawns initially");
}

static void testAmbientTrafficSpawnOnTimer() {
    std::cout << "\n=== Ambient Traffic: Spawns On Timer ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 5.0f;  // fast for testing

    auto* sys = world.createEntity("traffic_eco");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 1.0f;  // about to fire
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;
    state->resource_availability = 0.7f;
    state->pirate_activity = 0.5f;
    state->security_level = 0.6f;
    state->trade_volume = 0.6f;

    atSys.update(2.0f);  // timer fires
    auto spawns = atSys.getPendingSpawns("traffic_eco");
    assertTrue(!spawns.empty(), "Spawns generated after timer fires");
    assertTrue(traffic->active_traffic_count > 0, "Active traffic count increased");
}

static void testAmbientTrafficTraderSpawn() {
    std::cout << "\n=== Ambient Traffic: Trader Spawn on Good Economy ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_trader");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;
    state->resource_availability = 0.0f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_trader");
    bool has_trader = false;
    for (auto& s : spawns) if (s == "trader") has_trader = true;
    assertTrue(has_trader, "Trader spawned in good economy");
}

static void testAmbientTrafficMinerSpawn() {
    std::cout << "\n=== Ambient Traffic: Miner Spawn on Rich Resources ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_miner");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.0f;
    state->resource_availability = 0.9f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_miner");
    bool has_miner = false;
    for (auto& s : spawns) if (s == "miner") has_miner = true;
    assertTrue(has_miner, "Miner spawned in resource-rich system");
}

static void testAmbientTrafficCapReached() {
    std::cout << "\n=== Ambient Traffic: No Spawn When Cap Reached ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;
    atSys.max_traffic_per_system = 5;

    auto* sys = world.createEntity("traffic_full");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    traffic->active_traffic_count = 5;  // already at cap
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 1.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_full");
    assertTrue(spawns.empty(), "No spawns when at traffic cap");
}

static void testAmbientTrafficClearPending() {
    std::cout << "\n=== Ambient Traffic: Clear Pending Spawns ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_clear");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;

    atSys.update(1.0f);
    assertTrue(!traffic->pending_spawns.empty(), "Pending spawns exist");

    atSys.clearPendingSpawns("traffic_clear");
    assertTrue(traffic->pending_spawns.empty(), "Pending spawns cleared");
}

static void testAmbientTrafficMissingEntity() {
    std::cout << "\n=== Ambient Traffic: Missing Entity Queries ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);

    auto spawns = atSys.getPendingSpawns("nobody");
    assertTrue(spawns.empty(), "No spawns for missing entity");
    assertTrue(atSys.getActiveTrafficCount("nobody") == 0, "Zero traffic for missing entity");
}


void run_ambient_traffic_system_tests() {
    testAmbientTrafficDefaults();
    testAmbientTrafficSpawnOnTimer();
    testAmbientTrafficTraderSpawn();
    testAmbientTrafficMinerSpawn();
    testAmbientTrafficCapReached();
    testAmbientTrafficClearPending();
    testAmbientTrafficMissingEntity();
}
