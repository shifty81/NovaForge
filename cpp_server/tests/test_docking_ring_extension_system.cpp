// Tests for: Docking Ring Extension System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/docking_ring_extension_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Docking Ring Extension System Tests ====================

static void testDockingRingInit() {
    std::cout << "\n=== Docking Ring: Init ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    auto* entity = world.createEntity("test_ship");
    assertTrue(sys.initializeRing("test_ship", 12.0f), "Ring initialized");
    auto* ring = entity->getComponent<components::DockingRingExtension>();
    assertTrue(ring != nullptr, "Ring component exists");
    assertTrue(approxEqual(ring->ring_diameter, 12.0f), "Ring diameter set");
    assertTrue(ring->state == components::DockingRingExtension::RingState::Retracted, "Initial state retracted");
    assertTrue(!sys.initializeRing("test_ship", 10.0f), "Duplicate init fails");
}

static void testDockingRingExtend() {
    std::cout << "\n=== Docking Ring: Extend ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    assertTrue(sys.extendRing("test_ship"), "Extend starts");
    assertTrue(sys.getState("test_ship") == "extending", "State is extending");
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getProgress("test_ship"), 0.5f), "Progress 0.5 after 1s");
    sys.update(1.0f);
    assertTrue(sys.getState("test_ship") == "extended", "State extended after 2s");
    assertTrue(approxEqual(sys.getProgress("test_ship"), 1.0f), "Progress 1.0 when extended");
}

static void testDockingRingRetract() {
    std::cout << "\n=== Docking Ring: Retract ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    assertTrue(sys.retractRing("test_ship"), "Retract starts");
    assertTrue(sys.getState("test_ship") == "retracting", "State is retracting");
    sys.update(2.0f);
    assertTrue(sys.getState("test_ship") == "retracted", "State retracted after 2s");
}

static void testDockingRingConnect() {
    std::cout << "\n=== Docking Ring: Connect ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 3.0f);
    assertTrue(sys.connectRing("test_ship", "target_ship", components::DockingRingExtension::ConnectionType::ShipToShip), "Connect succeeds");
    assertTrue(sys.isConnected("test_ship"), "Is connected");
}

static void testDockingRingDisconnect() {
    std::cout << "\n=== Docking Ring: Disconnect ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 2.0f);
    sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip);
    assertTrue(sys.disconnectRing("test_ship"), "Disconnect succeeds");
    assertTrue(!sys.isConnected("test_ship"), "No longer connected");
    assertTrue(!sys.disconnectRing("test_ship"), "Double disconnect fails");
}

static void testDockingRingAlignment() {
    std::cout << "\n=== Docking Ring: Alignment ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 10.0f);
    assertTrue(!sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip), "Connect fails with bad alignment");
    sys.setAlignment("test_ship", 4.0f);
    assertTrue(sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip), "Connect succeeds with good alignment");
}

static void testDockingRingPressure() {
    std::cout << "\n=== Docking Ring: Pressure ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    assertTrue(!sys.sealPressure("test_ship"), "Cannot seal when retracted");
    sys.extendRing("test_ship");
    sys.update(2.0f);
    assertTrue(sys.sealPressure("test_ship"), "Seal when extended");
    assertTrue(!sys.sealPressure("test_ship"), "Cannot double seal");
    assertTrue(sys.unsealPressure("test_ship"), "Unseal succeeds");
}

static void testDockingRingPower() {
    std::cout << "\n=== Docking Ring: Power ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.setPowerEnabled("test_ship", false);
    assertTrue(!sys.extendRing("test_ship"), "Cannot extend without power");
    sys.setPowerEnabled("test_ship", true);
    assertTrue(sys.extendRing("test_ship"), "Can extend with power");
}

static void testDockingRingIntegrity() {
    std::cout << "\n=== Docking Ring: Integrity ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    assertTrue(approxEqual(sys.getIntegrity("test_ship"), 1.0f), "Initial integrity 1.0");
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 0.0f);
    sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip);
    sys.update(10.0f);
    assertTrue(sys.getIntegrity("test_ship") < 1.0f, "Integrity degrades when connected");
    sys.repairRing("test_ship", 1.0f);
    assertTrue(approxEqual(sys.getIntegrity("test_ship"), 1.0f), "Repair restores integrity");
}

static void testDockingRingMissing() {
    std::cout << "\n=== Docking Ring: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    assertTrue(!sys.initializeRing("nonexistent", 10.0f), "Init fails on missing");
    assertTrue(!sys.extendRing("nonexistent"), "Extend fails on missing");
    assertTrue(sys.getState("nonexistent") == "unknown", "State unknown on missing");
    assertTrue(approxEqual(sys.getProgress("nonexistent"), 0.0f), "Progress 0 on missing");
}


void run_docking_ring_extension_system_tests() {
    testDockingRingInit();
    testDockingRingExtend();
    testDockingRingRetract();
    testDockingRingConnect();
    testDockingRingDisconnect();
    testDockingRingAlignment();
    testDockingRingPressure();
    testDockingRingPower();
    testDockingRingIntegrity();
    testDockingRingMissing();
}
