// Tests for: System Traffic Monitor System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/system_traffic_monitor_system.h"

using namespace atlas;

// ==================== System Traffic Monitor System Tests ====================

static void testSystemTrafficMonitorCreate() {
    std::cout << "\n=== SystemTrafficMonitor: Create ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    assertTrue(sys.initialize("mon1", "sys_jita"), "Init succeeds");
    assertTrue(sys.getEntityCount("mon1") == 0, "No entities tracked initially");
    assertTrue(sys.getPlayerCount("mon1") == 0, "No players initially");
    assertTrue(sys.getNPCTraderCount("mon1") == 0, "No traders initially");
    assertTrue(sys.getNPCMinerCount("mon1") == 0, "No miners initially");
    assertTrue(sys.getNPCPirateCount("mon1") == 0, "No pirates initially");
    assertTrue(sys.getNPCSecurityCount("mon1") == 0, "No security initially");
    assertTrue(!sys.isCongested("mon1"), "Not congested initially");
    assertTrue(sys.getTotalSnapshots("mon1") == 0, "No snapshots initially");
    assertTrue(sys.getTotalEntitiesTracked("mon1") == 0, "No total entities");
}

static void testSystemTrafficMonitorRegister() {
    std::cout << "\n=== SystemTrafficMonitor: Register ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");
    assertTrue(sys.registerEntity("mon1", "player1", 0), "Register player");
    assertTrue(sys.registerEntity("mon1", "trader1", 1), "Register trader");
    assertTrue(sys.registerEntity("mon1", "miner1", 2), "Register miner");
    assertTrue(sys.registerEntity("mon1", "pirate1", 3), "Register pirate");
    assertTrue(sys.registerEntity("mon1", "security1", 4), "Register security");
    assertTrue(sys.getEntityCount("mon1") == 5, "5 entities tracked");
    assertTrue(sys.getPlayerCount("mon1") == 1, "1 player");
    assertTrue(sys.getNPCTraderCount("mon1") == 1, "1 trader");
    assertTrue(sys.getNPCMinerCount("mon1") == 1, "1 miner");
    assertTrue(sys.getNPCPirateCount("mon1") == 1, "1 pirate");
    assertTrue(sys.getNPCSecurityCount("mon1") == 1, "1 security");
    assertTrue(sys.getTotalEntitiesTracked("mon1") == 5, "5 total entities tracked");
}

static void testSystemTrafficMonitorDuplicate() {
    std::cout << "\n=== SystemTrafficMonitor: Duplicate ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");
    sys.registerEntity("mon1", "player1", 0);
    assertTrue(!sys.registerEntity("mon1", "player1", 1), "Duplicate rejected");
    assertTrue(sys.getEntityCount("mon1") == 1, "Still 1 entity");
}

static void testSystemTrafficMonitorRemove() {
    std::cout << "\n=== SystemTrafficMonitor: Remove ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");
    sys.registerEntity("mon1", "player1", 0);
    sys.registerEntity("mon1", "trader1", 1);
    assertTrue(sys.removeEntity("mon1", "player1"), "Remove player");
    assertTrue(sys.getEntityCount("mon1") == 1, "1 entity after remove");
    assertTrue(sys.getPlayerCount("mon1") == 0, "0 players after remove");
    assertTrue(!sys.removeEntity("mon1", "player1"), "Double remove fails");
}

static void testSystemTrafficMonitorCongestion() {
    std::cout << "\n=== SystemTrafficMonitor: Congestion ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");

    auto* entity = world.getEntity("mon1");
    auto* comp = entity->getComponent<components::SystemTrafficMonitor>();
    comp->congestion_threshold = 3.0f;

    sys.registerEntity("mon1", "e1", 0);
    sys.registerEntity("mon1", "e2", 1);
    sys.update(0.1f);
    assertTrue(!sys.isCongested("mon1"), "Not congested with 2 entities");

    sys.registerEntity("mon1", "e3", 2);
    sys.update(0.1f);
    assertTrue(sys.isCongested("mon1"), "Congested with 3 entities (threshold 3)");
}

static void testSystemTrafficMonitorSnapshot() {
    std::cout << "\n=== SystemTrafficMonitor: Snapshot ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");

    auto* entity = world.getEntity("mon1");
    auto* comp = entity->getComponent<components::SystemTrafficMonitor>();
    comp->snapshot_interval = 10.0f;

    sys.update(5.0f);
    assertTrue(sys.getTotalSnapshots("mon1") == 0, "No snapshot before interval");
    sys.update(6.0f); // 11s total, past 10s interval
    assertTrue(sys.getTotalSnapshots("mon1") == 1, "1 snapshot after interval");
    sys.update(10.0f); // 21s total
    assertTrue(sys.getTotalSnapshots("mon1") == 2, "2 snapshots after double interval");
}

static void testSystemTrafficMonitorTimeInSystem() {
    std::cout << "\n=== SystemTrafficMonitor: TimeInSystem ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");
    sys.registerEntity("mon1", "player1", 0);

    sys.update(5.0f);
    auto* entity = world.getEntity("mon1");
    auto* comp = entity->getComponent<components::SystemTrafficMonitor>();
    assertTrue(approxEqual(comp->entries[0].time_in_system, 5.0f), "Time in system is 5s");
}

static void testSystemTrafficMonitorMaxEntries() {
    std::cout << "\n=== SystemTrafficMonitor: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");

    auto* entity = world.getEntity("mon1");
    auto* comp = entity->getComponent<components::SystemTrafficMonitor>();
    comp->max_entries = 2;

    sys.registerEntity("mon1", "e1", 0);
    sys.registerEntity("mon1", "e2", 1);
    assertTrue(!sys.registerEntity("mon1", "e3", 2), "Max entries enforced");
    assertTrue(sys.getEntityCount("mon1") == 2, "Still 2 entities");
}

static void testSystemTrafficMonitorMultipleCategories() {
    std::cout << "\n=== SystemTrafficMonitor: MultipleCategories ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    world.createEntity("mon1");
    sys.initialize("mon1", "sys_jita");

    sys.registerEntity("mon1", "p1", 0);
    sys.registerEntity("mon1", "p2", 0);
    sys.registerEntity("mon1", "t1", 1);
    sys.registerEntity("mon1", "m1", 2);
    sys.registerEntity("mon1", "m2", 2);
    sys.registerEntity("mon1", "m3", 2);
    sys.registerEntity("mon1", "pi1", 3);

    assertTrue(sys.getPlayerCount("mon1") == 2, "2 players");
    assertTrue(sys.getNPCTraderCount("mon1") == 1, "1 trader");
    assertTrue(sys.getNPCMinerCount("mon1") == 3, "3 miners");
    assertTrue(sys.getNPCPirateCount("mon1") == 1, "1 pirate");
    assertTrue(sys.getNPCSecurityCount("mon1") == 0, "0 security");
    assertTrue(sys.getEntityCount("mon1") == 7, "7 total entities");
}

static void testSystemTrafficMonitorMissing() {
    std::cout << "\n=== SystemTrafficMonitor: Missing ===" << std::endl;
    ecs::World world;
    systems::SystemTrafficMonitorSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "sys_jita"), "Init fails on missing");
    assertTrue(!sys.registerEntity("nonexistent", "e1", 0), "Register fails on missing");
    assertTrue(!sys.removeEntity("nonexistent", "e1"), "Remove fails on missing");
    assertTrue(sys.getEntityCount("nonexistent") == 0, "0 entities on missing");
    assertTrue(sys.getPlayerCount("nonexistent") == 0, "0 players on missing");
    assertTrue(sys.getNPCTraderCount("nonexistent") == 0, "0 traders on missing");
    assertTrue(sys.getNPCMinerCount("nonexistent") == 0, "0 miners on missing");
    assertTrue(sys.getNPCPirateCount("nonexistent") == 0, "0 pirates on missing");
    assertTrue(sys.getNPCSecurityCount("nonexistent") == 0, "0 security on missing");
    assertTrue(!sys.isCongested("nonexistent"), "Not congested on missing");
    assertTrue(sys.getTotalSnapshots("nonexistent") == 0, "0 snapshots on missing");
    assertTrue(sys.getTotalEntitiesTracked("nonexistent") == 0, "0 total on missing");
}


void run_system_traffic_monitor_system_tests() {
    testSystemTrafficMonitorCreate();
    testSystemTrafficMonitorRegister();
    testSystemTrafficMonitorDuplicate();
    testSystemTrafficMonitorRemove();
    testSystemTrafficMonitorCongestion();
    testSystemTrafficMonitorSnapshot();
    testSystemTrafficMonitorTimeInSystem();
    testSystemTrafficMonitorMaxEntries();
    testSystemTrafficMonitorMultipleCategories();
    testSystemTrafficMonitorMissing();
}
