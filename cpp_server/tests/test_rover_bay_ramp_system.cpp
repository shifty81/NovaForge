// Tests for: Rover Bay Ramp System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/rover_bay_ramp_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Rover Bay Ramp System Tests ====================

static void testRoverBayInit() {
    std::cout << "\n=== Rover Bay: Init ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    auto* entity = world.createEntity("test_ship");
    assertTrue(sys.initializeBay("test_ship", 3), "Bay initialized");
    auto* bay = entity->getComponent<components::RoverBayRamp>();
    assertTrue(bay != nullptr, "Bay component exists");
    assertTrue(bay->max_rovers == 3, "Max rovers set");
    assertTrue(bay->state == components::RoverBayRamp::RampState::Closed, "Initial state closed");
    assertTrue(!sys.initializeBay("test_ship", 2), "Duplicate init fails");
}

static void testRoverBayOpen() {
    std::cout << "\n=== Rover Bay: Open ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    assertTrue(sys.openRamp("test_ship"), "Open starts");
    assertTrue(sys.getRampState("test_ship") == "opening", "State is opening");
    sys.update(2.0f);
    assertTrue(approxEqual(sys.getRampProgress("test_ship"), 0.6f), "Progress 0.6 after 2s");
    sys.update(2.0f);
    assertTrue(sys.getRampState("test_ship") == "open", "State open after 4s");
}

static void testRoverBayClose() {
    std::cout << "\n=== Rover Bay: Close ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.openRamp("test_ship");
    sys.update(4.0f);
    assertTrue(sys.closeRamp("test_ship"), "Close starts");
    assertTrue(sys.getRampState("test_ship") == "closing", "State is closing");
    sys.update(4.0f);
    assertTrue(sys.getRampState("test_ship") == "closed", "State closed after 4s");
}

static void testRoverBayStore() {
    std::cout << "\n=== Rover Bay: Store ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    assertTrue(!sys.storeRover("test_ship", "rover1"), "Cannot store when closed");
    sys.openRamp("test_ship");
    sys.update(4.0f);
    assertTrue(sys.storeRover("test_ship", "rover1"), "Store rover1");
    assertTrue(sys.storeRover("test_ship", "rover2"), "Store rover2");
    assertTrue(!sys.storeRover("test_ship", "rover3"), "Cannot exceed capacity");
    assertTrue(sys.getStoredCount("test_ship") == 2, "Stored count is 2");
}

static void testRoverBayDeploy() {
    std::cout << "\n=== Rover Bay: Deploy ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.openRamp("test_ship");
    sys.update(4.0f);
    sys.storeRover("test_ship", "rover1");
    assertTrue(sys.deployRover("test_ship", "rover1"), "Deploy rover1");
    assertTrue(sys.getStoredCount("test_ship") == 0, "Stored count 0 after deploy");
    assertTrue(sys.getDeployedCount("test_ship") == 1, "Deployed count 1");
    assertTrue(!sys.deployRover("test_ship", "rover1"), "Cannot deploy already deployed");
}

static void testRoverBayRetrieve() {
    std::cout << "\n=== Rover Bay: Retrieve ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.openRamp("test_ship");
    sys.update(4.0f);
    sys.storeRover("test_ship", "rover1");
    sys.deployRover("test_ship", "rover1");
    assertTrue(sys.retrieveRover("test_ship", "rover1"), "Retrieve rover1");
    assertTrue(sys.getStoredCount("test_ship") == 1, "Stored count 1 after retrieve");
    assertTrue(sys.getDeployedCount("test_ship") == 0, "Deployed count 0 after retrieve");
}

static void testRoverBaySafety() {
    std::cout << "\n=== Rover Bay: Safety ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.setExternalAtmosphere("test_ship", components::RoverBayRamp::AtmosphereType::Corrosive);
    assertTrue(!sys.openRamp("test_ship"), "Cannot open in corrosive atmosphere");
    sys.setExternalAtmosphere("test_ship", components::RoverBayRamp::AtmosphereType::Toxic);
    assertTrue(sys.openRamp("test_ship"), "Can open in toxic atmosphere");
}

static void testRoverBayPower() {
    std::cout << "\n=== Rover Bay: Power ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.setPowerEnabled("test_ship", false);
    assertTrue(!sys.openRamp("test_ship"), "Cannot open without power");
    sys.setPowerEnabled("test_ship", true);
    assertTrue(sys.openRamp("test_ship"), "Can open with power");
}

static void testRoverBayAtmosphere() {
    std::cout << "\n=== Rover Bay: Atmosphere ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    auto* entity = world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    auto* bay = entity->getComponent<components::RoverBayRamp>();
    assertTrue(bay->is_pressurized, "Initially pressurized");
    sys.openRamp("test_ship");
    sys.update(4.0f);
    assertTrue(!bay->is_pressurized, "Depressurized when open");
    sys.closeRamp("test_ship");
    sys.update(4.0f);
    assertTrue(bay->is_pressurized, "Repressurized when closed");
}

static void testRoverBayMissing() {
    std::cout << "\n=== Rover Bay: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    assertTrue(!sys.initializeBay("nonexistent", 2), "Init fails on missing");
    assertTrue(!sys.openRamp("nonexistent"), "Open fails on missing");
    assertTrue(sys.getRampState("nonexistent") == "unknown", "State unknown on missing");
    assertTrue(sys.getStoredCount("nonexistent") == 0, "Stored 0 on missing");
}


void run_rover_bay_ramp_system_tests() {
    testRoverBayInit();
    testRoverBayOpen();
    testRoverBayClose();
    testRoverBayStore();
    testRoverBayDeploy();
    testRoverBayRetrieve();
    testRoverBaySafety();
    testRoverBayPower();
    testRoverBayAtmosphere();
    testRoverBayMissing();
}
