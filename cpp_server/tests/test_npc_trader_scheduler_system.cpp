// Tests for: NPC Trader Scheduler System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/npc_trader_scheduler_system.h"

using namespace atlas;

// ==================== NPC Trader Scheduler System Tests ====================

static void testNpcTraderCreate() {
    std::cout << "\n=== NpcTraderScheduler: Create ===" << std::endl;
    ecs::World world;
    systems::NpcTraderSchedulerSystem sys(&world);
    world.createEntity("sched1");
    assertTrue(sys.initialize("sched1", "scheduler_alpha"), "Init succeeds");
    assertTrue(sys.getActiveHaulers("sched1") == 0, "No active haulers");
    assertTrue(sys.getRouteCount("sched1") == 0, "No routes");
    assertTrue(sys.getTotalDeliveries("sched1") == 0, "No deliveries");
    assertTrue(sys.getTotalCargoValueDelivered("sched1") == 0.0, "0 cargo value");
}

static void testNpcTraderAddRoute() {
    std::cout << "\n=== NpcTraderScheduler: AddRoute ===" << std::endl;
    ecs::World world;
    systems::NpcTraderSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.initialize("sched1", "scheduler_alpha");

    assertTrue(sys.addRoute("sched1", "route1", "station_a", "station_b",
                           "ore", 500.0f, 5000.0), "Add route succeeds");
    assertTrue(sys.getRouteCount("sched1") == 1, "1 route");
    assertTrue(sys.getRouteState("sched1", "route1") == "Waiting", "Route waiting");

    assertTrue(sys.addRoute("sched1", "route2", "station_b", "station_c",
                           "goods", 200.0f, 3000.0), "Add second route");
    assertTrue(sys.getRouteCount("sched1") == 2, "2 routes");
}

static void testNpcTraderSpawnCycle() {
    std::cout << "\n=== NpcTraderScheduler: SpawnCycle ===" << std::endl;
    ecs::World world;
    systems::NpcTraderSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.initialize("sched1", "scheduler_alpha");
    sys.setSpawnInterval("sched1", 10.0f);
    sys.addRoute("sched1", "route1", "station_a", "station_b", "ore", 100.0f, 1000.0);

    // Before spawn interval, route stays waiting
    sys.update(5.0f);
    assertTrue(sys.getRouteState("sched1", "route1") == "Waiting", "Still waiting at 5s");
    assertTrue(sys.getActiveHaulers("sched1") == 0, "No haulers yet");

    // After spawn interval, hauler spawns and route starts loading
    sys.update(6.0f);  // total 11s, past 10s interval
    assertTrue(sys.getRouteState("sched1", "route1") == "Loading", "Loading after spawn");
    assertTrue(sys.getActiveHaulers("sched1") == 1, "1 active hauler");
}

static void testNpcTraderFullDelivery() {
    std::cout << "\n=== NpcTraderScheduler: FullDelivery ===" << std::endl;
    ecs::World world;
    systems::NpcTraderSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.initialize("sched1", "scheduler_alpha");
    sys.setSpawnInterval("sched1", 1.0f);  // fast spawn
    sys.addRoute("sched1", "route1", "station_a", "station_b", "ore", 100.0f, 5000.0);

    // Trigger spawn
    sys.update(2.0f);
    assertTrue(sys.getRouteState("sched1", "route1") == "Loading", "Loading");

    // Complete loading (default load_time = 10s)
    sys.update(15.0f);
    assertTrue(sys.getRouteState("sched1", "route1") == "InTransit", "In transit");

    // Complete transit (default travel_time = 120s)
    sys.update(130.0f);
    assertTrue(sys.getRouteState("sched1", "route1") == "Unloading", "Unloading");

    // Complete unloading
    sys.update(15.0f);

    // After one more tick, it should be Complete and then reset to Waiting
    sys.update(0.1f);
    assertTrue(sys.getTotalDeliveries("sched1") >= 1, "At least 1 delivery");
    assertTrue(sys.getTotalCargoValueDelivered("sched1") >= 5000.0, "5000 ISC delivered");
}

static void testNpcTraderMaxHaulers() {
    std::cout << "\n=== NpcTraderScheduler: MaxHaulers ===" << std::endl;
    ecs::World world;
    systems::NpcTraderSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.initialize("sched1", "scheduler_alpha");
    sys.setMaxHaulers("sched1", 2);
    sys.setSpawnInterval("sched1", 1.0f);

    // Add 5 routes but only 2 haulers allowed
    for (int i = 0; i < 5; i++) {
        sys.addRoute("sched1", "route" + std::to_string(i), "a", "b",
                    "ore", 100.0f, 1000.0);
    }

    sys.update(2.0f);  // trigger spawn
    assertTrue(sys.getActiveHaulers("sched1") <= 2, "Max 2 haulers");
}

static void testNpcTraderMissing() {
    std::cout << "\n=== NpcTraderScheduler: Missing ===" << std::endl;
    ecs::World world;
    systems::NpcTraderSchedulerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "sched"), "Init fails on missing");
    assertTrue(!sys.addRoute("nonexistent", "r", "a", "b", "ore", 1.0f, 1.0),
               "AddRoute fails on missing");
    assertTrue(!sys.setMaxHaulers("nonexistent", 5), "SetMaxHaulers fails on missing");
    assertTrue(!sys.setSpawnInterval("nonexistent", 10.0f), "SetSpawnInterval fails on missing");
    assertTrue(sys.getActiveHaulers("nonexistent") == 0, "0 haulers on missing");
    assertTrue(sys.getRouteCount("nonexistent") == 0, "0 routes on missing");
    assertTrue(sys.getTotalDeliveries("nonexistent") == 0, "0 deliveries on missing");
    assertTrue(sys.getTotalCargoValueDelivered("nonexistent") == 0.0, "0 value on missing");
    assertTrue(sys.getRouteState("nonexistent", "r") == "Unknown", "Unknown state on missing");
}

void run_npc_trader_scheduler_system_tests() {
    testNpcTraderCreate();
    testNpcTraderAddRoute();
    testNpcTraderSpawnCycle();
    testNpcTraderFullDelivery();
    testNpcTraderMaxHaulers();
    testNpcTraderMissing();
}
