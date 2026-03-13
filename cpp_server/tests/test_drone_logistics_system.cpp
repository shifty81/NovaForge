// Tests for: DroneLogisticsSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/drone_logistics_system.h"

using namespace atlas;

// ==================== DroneLogisticsSystem Tests ====================

static void testDroneLogisticsInit() {
    std::cout << "\n=== DroneLogistics: Init ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_request_count("e1") == 0, "No requests");
    assertTrue(sys.get_pending_count("e1") == 0, "No pending");
    assertTrue(!sys.is_fleet_deploy_mode("e1"), "Fleet deploy off");
    assertTrue(sys.get_total_transfers_completed("e1") == 0, "Zero completed");
    assertTrue(sys.get_max_drones("e1") == 5, "Default max drones 5");
    assertTrue(sys.get_active_drones("e1") == 0, "Zero active drones");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testDroneLogisticsQueue() {
    std::cout << "\n=== DroneLogistics: Queue ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.queue_transfer("e1", "r1", "portA", "portB", "ore", 100), "Queue r1");
    assertTrue(sys.queue_transfer("e1", "r2", "portA", "portC", "minerals", 50), "Queue r2");
    assertTrue(sys.get_request_count("e1") == 2, "Two requests");
    assertTrue(sys.get_pending_count("e1") == 2, "Two pending");

    // Duplicate
    assertTrue(!sys.queue_transfer("e1", "r1", "x", "y", "z", 10), "Duplicate rejected");
    // Empty id
    assertTrue(!sys.queue_transfer("e1", "", "x", "y", "z", 10), "Empty id rejected");
    // Zero amount
    assertTrue(!sys.queue_transfer("e1", "r3", "x", "y", "z", 0), "Zero amount rejected");
    // Negative amount
    assertTrue(!sys.queue_transfer("e1", "r4", "x", "y", "z", -5), "Negative amount rejected");
}

static void testDroneLogisticsComplete() {
    std::cout << "\n=== DroneLogistics: Complete ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.queue_transfer("e1", "r1", "a", "b", "ore", 100);

    assertTrue(sys.complete_transfer("e1", "r1"), "Complete r1");
    assertTrue(sys.get_pending_count("e1") == 0, "Zero pending after complete");
    assertTrue(sys.get_request_count("e1") == 1, "Request still in list");
    assertTrue(sys.get_total_transfers_completed("e1") == 1, "One completed");

    // Double complete
    assertTrue(!sys.complete_transfer("e1", "r1"), "Double complete rejected");
    // Nonexistent
    assertTrue(!sys.complete_transfer("e1", "r_ghost"), "Complete nonexistent fails");
}

static void testDroneLogisticsCancel() {
    std::cout << "\n=== DroneLogistics: Cancel ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.queue_transfer("e1", "r1", "a", "b", "ore", 100);
    sys.queue_transfer("e1", "r2", "a", "c", "gas", 50);

    assertTrue(sys.cancel_transfer("e1", "r1"), "Cancel r1");
    assertTrue(sys.get_request_count("e1") == 1, "One request left");
    assertTrue(!sys.cancel_transfer("e1", "r1"), "Cancel again fails");
    assertTrue(!sys.cancel_transfer("e1", "ghost"), "Cancel nonexistent fails");
}

static void testDroneLogisticsFleetDeploy() {
    std::cout << "\n=== DroneLogistics: FleetDeploy ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.queue_transfer("e1", "r1", "a", "b", "ore", 100);
    sys.queue_transfer("e1", "r2", "a", "c", "gas", 50);
    sys.queue_transfer("e1", "r3", "a", "d", "ice", 25);

    assertTrue(sys.set_fleet_deploy_mode("e1", true), "Enable fleet deploy");
    assertTrue(sys.is_fleet_deploy_mode("e1"), "Fleet deploy on");

    // Tick — should process one pending per tick
    sys.update(0.016f);
    assertTrue(sys.get_pending_count("e1") == 2, "Two pending after 1 tick");
    assertTrue(sys.get_total_transfers_completed("e1") == 1, "One completed after tick");
    assertTrue(sys.get_active_drones("e1") == 1, "One active drone");

    sys.update(0.016f);
    assertTrue(sys.get_pending_count("e1") == 1, "One pending after 2 ticks");
    assertTrue(sys.get_active_drones("e1") == 2, "Two active drones");

    sys.update(0.016f);
    assertTrue(sys.get_pending_count("e1") == 0, "Zero pending after 3 ticks");
    assertTrue(sys.get_total_transfers_completed("e1") == 3, "Three completed");
    assertTrue(sys.get_active_drones("e1") == 3, "Three active drones");

    // No more pending — tick should not crash or change counts
    sys.update(0.016f);
    assertTrue(sys.get_total_transfers_completed("e1") == 3, "Still three completed");
}

static void testDroneLogisticsMaxDrones() {
    std::cout << "\n=== DroneLogistics: MaxDrones ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.set_max_drones("e1", 2), "Set max drones to 2");
    assertTrue(sys.get_max_drones("e1") == 2, "Max drones is 2");

    // Queue 5 requests
    for (int i = 0; i < 5; i++) {
        sys.queue_transfer("e1", "r" + std::to_string(i), "a", "b", "ore", 10);
    }

    sys.set_fleet_deploy_mode("e1", true);
    sys.update(0.016f);
    sys.update(0.016f);
    assertTrue(sys.get_active_drones("e1") == 2, "Active drones capped at 2");

    // Third tick should not process because active == max
    sys.update(0.016f);
    assertTrue(sys.get_active_drones("e1") == 2, "Still capped at 2");
    assertTrue(sys.get_total_transfers_completed("e1") == 2, "Only 2 completed");
}

static void testDroneLogisticsClear() {
    std::cout << "\n=== DroneLogistics: Clear ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.queue_transfer("e1", "r1", "a", "b", "ore", 100);
    sys.queue_transfer("e1", "r2", "a", "c", "gas", 50);

    assertTrue(sys.clear_requests("e1"), "Clear requests");
    assertTrue(sys.get_request_count("e1") == 0, "Zero requests after clear");
    assertTrue(sys.get_pending_count("e1") == 0, "Zero pending after clear");
}

static void testDroneLogisticsSetMaxDronesValidation() {
    std::cout << "\n=== DroneLogistics: SetMaxDronesValidation ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.set_max_drones("e1", -1), "Negative max drones rejected");
    assertTrue(sys.set_max_drones("e1", 0), "Zero max drones allowed");
    assertTrue(sys.get_max_drones("e1") == 0, "Max drones is 0");
}

static void testDroneLogisticsMissing() {
    std::cout << "\n=== DroneLogistics: Missing ===" << std::endl;
    ecs::World world;
    systems::DroneLogisticsSystem sys(&world);

    assertTrue(!sys.queue_transfer("no", "r", "a", "b", "o", 1), "queue fails");
    assertTrue(!sys.complete_transfer("no", "r"), "complete fails");
    assertTrue(!sys.cancel_transfer("no", "r"), "cancel fails");
    assertTrue(!sys.clear_requests("no"), "clear fails");
    assertTrue(!sys.set_fleet_deploy_mode("no", true), "set_fleet_deploy fails");
    assertTrue(!sys.set_max_drones("no", 1), "set_max_drones fails");
    assertTrue(sys.get_request_count("no") == 0, "get_request_count default");
    assertTrue(sys.get_pending_count("no") == 0, "get_pending_count default");
    assertTrue(!sys.is_fleet_deploy_mode("no"), "is_fleet_deploy default");
    assertTrue(sys.get_total_transfers_completed("no") == 0, "completed default");
    assertTrue(sys.get_max_drones("no") == 0, "max_drones default");
    assertTrue(sys.get_active_drones("no") == 0, "active_drones default");
}

void run_drone_logistics_system_tests() {
    testDroneLogisticsInit();
    testDroneLogisticsQueue();
    testDroneLogisticsComplete();
    testDroneLogisticsCancel();
    testDroneLogisticsFleetDeploy();
    testDroneLogisticsMaxDrones();
    testDroneLogisticsClear();
    testDroneLogisticsSetMaxDronesValidation();
    testDroneLogisticsMissing();
}
