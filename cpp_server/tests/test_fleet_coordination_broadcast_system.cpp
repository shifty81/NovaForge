// Tests for: Fleet Coordination Broadcast System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fleet_coordination_broadcast_system.h"

using namespace atlas;

// ==================== Fleet Coordination Broadcast System Tests ====================

static void testFleetCoordinationBroadcastCreate() {
    std::cout << "\n=== FleetCoordinationBroadcast: Create ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    assertTrue(sys.initialize("fcb1"), "Init succeeds");
    assertTrue(sys.getActiveSignalCount("fcb1") == 0, "No active signals initially");
    assertTrue(approxEqual(sys.getBroadcastRange("fcb1"), 100.0f), "Default range is 100");
    assertTrue(approxEqual(sys.getSignalStrength("fcb1"), 1.0f), "Default strength is 1.0");
    assertTrue(sys.getTotalBroadcasts("fcb1") == 0, "0 broadcasts initially");
    assertTrue(sys.getTotalAcknowledged("fcb1") == 0, "0 acknowledged initially");
}

static void testFleetCoordinationBroadcastSignal() {
    std::cout << "\n=== FleetCoordinationBroadcast: Signal ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    assertTrue(sys.broadcastSignal("fcb1", "rally", "commander1"), "Broadcast rally");
    assertTrue(sys.getActiveSignalCount("fcb1") == 1, "1 active signal");
    assertTrue(sys.hasActiveSignal("fcb1", "rally"), "Rally is active");
    assertTrue(!sys.hasActiveSignal("fcb1", "retreat"), "Retreat is not active");
    assertTrue(sys.getTotalBroadcasts("fcb1") == 1, "1 total broadcast");
}

static void testFleetCoordinationBroadcastAllTypes() {
    std::cout << "\n=== FleetCoordinationBroadcast: AllTypes ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    assertTrue(sys.broadcastSignal("fcb1", "rally", "cmd"), "Rally ok");
    assertTrue(sys.broadcastSignal("fcb1", "retreat", "cmd"), "Retreat ok");
    assertTrue(sys.broadcastSignal("fcb1", "regroup", "cmd"), "Regroup ok");
    assertTrue(sys.broadcastSignal("fcb1", "hold", "cmd"), "Hold ok");
    assertTrue(sys.broadcastSignal("fcb1", "advance", "cmd"), "Advance ok");
    assertTrue(sys.getActiveSignalCount("fcb1") == 5, "5 active signals");
    assertTrue(!sys.broadcastSignal("fcb1", "invalid", "cmd"), "Invalid type rejected");
    assertTrue(!sys.broadcastSignal("fcb1", "rally", ""), "Empty issuer rejected");
}

static void testFleetCoordinationBroadcastAcknowledge() {
    std::cout << "\n=== FleetCoordinationBroadcast: Acknowledge ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    sys.broadcastSignal("fcb1", "rally", "cmd");
    assertTrue(sys.acknowledgeSignal("fcb1", "rally"), "Acknowledge rally");
    assertTrue(sys.getTotalAcknowledged("fcb1") == 1, "1 acknowledged");
    assertTrue(!sys.acknowledgeSignal("fcb1", "retreat"), "Cannot ack non-existent signal");
}

static void testFleetCoordinationBroadcastClear() {
    std::cout << "\n=== FleetCoordinationBroadcast: Clear ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    sys.broadcastSignal("fcb1", "rally", "cmd");
    sys.broadcastSignal("fcb1", "retreat", "cmd");
    assertTrue(sys.getActiveSignalCount("fcb1") == 2, "2 signals before clear");
    assertTrue(sys.clearSignals("fcb1"), "Clear succeeds");
    assertTrue(sys.getActiveSignalCount("fcb1") == 0, "0 signals after clear");
}

static void testFleetCoordinationBroadcastRange() {
    std::cout << "\n=== FleetCoordinationBroadcast: Range ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    assertTrue(sys.setBroadcastRange("fcb1", 250.0f), "Set range 250");
    assertTrue(approxEqual(sys.getBroadcastRange("fcb1"), 250.0f), "Range is 250");
    sys.setBroadcastRange("fcb1", 1000.0f);  // should clamp to 500
    assertTrue(approxEqual(sys.getBroadcastRange("fcb1"), 500.0f), "Range clamped to 500");
    sys.setBroadcastRange("fcb1", -10.0f);  // should clamp to 0
    assertTrue(approxEqual(sys.getBroadcastRange("fcb1"), 0.0f), "Range clamped to 0");
}

static void testFleetCoordinationBroadcastStrength() {
    std::cout << "\n=== FleetCoordinationBroadcast: Strength ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    assertTrue(sys.setSignalStrength("fcb1", 0.5f), "Set strength 0.5");
    assertTrue(approxEqual(sys.getSignalStrength("fcb1"), 0.5f), "Strength is 0.5");
    sys.setSignalStrength("fcb1", 2.0f);  // should clamp to 1.0
    assertTrue(approxEqual(sys.getSignalStrength("fcb1"), 1.0f), "Strength clamped to 1.0");
}

static void testFleetCoordinationBroadcastExpiry() {
    std::cout << "\n=== FleetCoordinationBroadcast: Expiry ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    sys.broadcastSignal("fcb1", "rally", "cmd");
    assertTrue(sys.getActiveSignalCount("fcb1") == 1, "1 signal before expiry");
    // Advance past signal duration (30s)
    sys.update(31.0f);
    assertTrue(sys.getActiveSignalCount("fcb1") == 0, "Signal expired after 31s");
}

static void testFleetCoordinationBroadcastMaxSignals() {
    std::cout << "\n=== FleetCoordinationBroadcast: MaxSignals ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    world.createEntity("fcb1");
    sys.initialize("fcb1");
    for (int i = 0; i < 10; ++i) {
        assertTrue(sys.broadcastSignal("fcb1", "rally", "cmd" + std::to_string(i)), "Broadcast " + std::to_string(i));
    }
    assertTrue(!sys.broadcastSignal("fcb1", "rally", "overflow"), "11th signal rejected (max 10)");
    assertTrue(sys.getActiveSignalCount("fcb1") == 10, "10 signals at max");
}

static void testFleetCoordinationBroadcastMissing() {
    std::cout << "\n=== FleetCoordinationBroadcast: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationBroadcastSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.broadcastSignal("nonexistent", "rally", "cmd"), "Broadcast fails on missing");
    assertTrue(!sys.acknowledgeSignal("nonexistent", "rally"), "Ack fails on missing");
    assertTrue(!sys.clearSignals("nonexistent"), "Clear fails on missing");
    assertTrue(!sys.setBroadcastRange("nonexistent", 100.0f), "Range fails on missing");
    assertTrue(!sys.setSignalStrength("nonexistent", 1.0f), "Strength fails on missing");
    assertTrue(approxEqual(sys.getBroadcastRange("nonexistent"), 0.0f), "0 range on missing");
    assertTrue(approxEqual(sys.getSignalStrength("nonexistent"), 0.0f), "0 strength on missing");
    assertTrue(sys.getActiveSignalCount("nonexistent") == 0, "0 signals on missing");
    assertTrue(sys.getTotalBroadcasts("nonexistent") == 0, "0 broadcasts on missing");
    assertTrue(sys.getTotalAcknowledged("nonexistent") == 0, "0 acknowledged on missing");
    assertTrue(!sys.hasActiveSignal("nonexistent", "rally"), "No signal on missing");
}


void run_fleet_coordination_broadcast_system_tests() {
    testFleetCoordinationBroadcastCreate();
    testFleetCoordinationBroadcastSignal();
    testFleetCoordinationBroadcastAllTypes();
    testFleetCoordinationBroadcastAcknowledge();
    testFleetCoordinationBroadcastClear();
    testFleetCoordinationBroadcastRange();
    testFleetCoordinationBroadcastStrength();
    testFleetCoordinationBroadcastExpiry();
    testFleetCoordinationBroadcastMaxSignals();
    testFleetCoordinationBroadcastMissing();
}
