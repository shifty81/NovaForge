// Tests for: FleetMoraleResolution System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_morale_resolution_system.h"

using namespace atlas;

// ===== FleetMoraleResolution System Tests =====

static void testFleetMoraleResCreate() {
    std::cout << "\n=== FleetMoraleRes: Create ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initializeFleet("fleet1"), "Init fleet succeeds");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 50.0f), "Initial morale is 50");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 0.5f), "Initial ideology is 0.5");
    assertTrue(!sys.isCrisisActive("fleet1"), "No crisis initially");
    assertTrue(sys.getDepartures("fleet1") == 0, "No departures initially");
    assertTrue(sys.getResolutionCount("fleet1") == 0, "No resolutions initially");
}

static void testFleetMoraleResCrisis() {
    std::cout << "\n=== FleetMoraleRes: Crisis ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    assertTrue(sys.triggerCrisis("fleet1"), "Trigger crisis succeeds");
    assertTrue(sys.isCrisisActive("fleet1"), "Crisis is active");
}

static void testFleetMoraleResResolve() {
    std::cout << "\n=== FleetMoraleRes: Resolve ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.triggerCrisis("fleet1");
    assertTrue(sys.resolveWithMethod("fleet1", "Compromise"), "Resolve with Compromise");
    assertTrue(!sys.isCrisisActive("fleet1"), "Crisis resolved");
    assertTrue(sys.getResolutionCount("fleet1") == 1, "Resolution count is 1");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 60.0f), "Morale boosted by Compromise (+10)");

    sys.triggerCrisis("fleet1");
    assertTrue(sys.resolveWithMethod("fleet1", "Mediation"), "Resolve with Mediation");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 80.0f), "Morale boosted by Mediation (+20)");
    assertTrue(sys.getResolutionCount("fleet1") == 2, "Resolution count is 2");
    assertTrue(!sys.resolveWithMethod("fleet1", "InvalidMethod"), "Invalid method fails");
}

static void testFleetMoraleResAdjust() {
    std::cout << "\n=== FleetMoraleRes: Adjust ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    assertTrue(sys.adjustMorale("fleet1", 30.0f), "Adjust morale up");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 80.0f), "Morale is 80");
    assertTrue(sys.adjustMorale("fleet1", 50.0f), "Adjust morale past max");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 100.0f), "Morale clamped at 100");
    assertTrue(sys.adjustMorale("fleet1", -150.0f), "Adjust morale below min");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 0.0f), "Morale clamped at 0");
}

static void testFleetMoraleResIdeology() {
    std::cout << "\n=== FleetMoraleRes: Ideology ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    assertTrue(sys.setIdeologyAlignment("fleet1", 0.8f), "Set ideology 0.8");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 0.8f), "Ideology is 0.8");
    assertTrue(sys.setIdeologyAlignment("fleet1", 2.0f), "Set ideology past max");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 1.0f), "Ideology clamped at 1.0");
    assertTrue(sys.setIdeologyAlignment("fleet1", -1.0f), "Set ideology below min");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 0.0f), "Ideology clamped at 0.0");
}

static void testFleetMoraleResRecovery() {
    std::cout << "\n=== FleetMoraleRes: Recovery ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.adjustMorale("fleet1", -20.0f); // morale = 30
    sys.setIdeologyAlignment("fleet1", 1.0f);
    sys.update(5.0f); // recovery = 1.0 * 5.0 * 1.0 = 5.0
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 35.0f), "Morale recovered to 35");
}

static void testFleetMoraleResAutoFracture() {
    std::cout << "\n=== FleetMoraleRes: AutoFracture ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.adjustMorale("fleet1", -35.0f); // morale = 15, below threshold of 20
    sys.update(0.1f); // should auto-trigger crisis
    assertTrue(sys.isCrisisActive("fleet1"), "Crisis auto-triggered below threshold");
}

static void testFleetMoraleResDeparture() {
    std::cout << "\n=== FleetMoraleRes: Departure ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.triggerCrisis("fleet1");
    sys.update(61.0f); // exceeds max_crisis_duration of 60
    assertTrue(sys.getDepartures("fleet1") == 1, "One departure after timeout");
    assertTrue(!sys.isCrisisActive("fleet1"), "Crisis deactivated after departure");
    assertTrue(sys.getFleetMorale("fleet1") < 50.0f, "Morale dropped after departure");
}

static void testFleetMoraleResMultiple() {
    std::cout << "\n=== FleetMoraleRes: Multiple ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.triggerCrisis("fleet1");
    sys.resolveWithMethod("fleet1", "Vote"); // +15
    sys.triggerCrisis("fleet1");
    sys.resolveWithMethod("fleet1", "AuthorityOverride"); // +5
    assertTrue(sys.getResolutionCount("fleet1") == 2, "Two resolutions applied");
    assertTrue(sys.getFleetMorale("fleet1") > 50.0f, "Morale increased from resolutions");
}

static void testFleetMoraleResMissing() {
    std::cout << "\n=== FleetMoraleRes: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    assertTrue(!sys.initializeFleet("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.triggerCrisis("nonexistent"), "Trigger fails on missing");
    assertTrue(!sys.resolveWithMethod("nonexistent", "Compromise"), "Resolve fails on missing");
    assertTrue(!sys.adjustMorale("nonexistent", 10.0f), "Adjust fails on missing");
    assertTrue(!sys.setIdeologyAlignment("nonexistent", 0.5f), "Set ideology fails on missing");
    assertTrue(approxEqual(sys.getFleetMorale("nonexistent"), 0.0f), "0 morale on missing");
    assertTrue(!sys.isCrisisActive("nonexistent"), "No crisis on missing");
    assertTrue(sys.getDepartures("nonexistent") == 0, "0 departures on missing");
    assertTrue(sys.getResolutionCount("nonexistent") == 0, "0 resolutions on missing");
}


void run_fleet_morale_resolution_system_tests() {
    testFleetMoraleResCreate();
    testFleetMoraleResCrisis();
    testFleetMoraleResResolve();
    testFleetMoraleResAdjust();
    testFleetMoraleResIdeology();
    testFleetMoraleResRecovery();
    testFleetMoraleResAutoFracture();
    testFleetMoraleResDeparture();
    testFleetMoraleResMultiple();
    testFleetMoraleResMissing();
}
