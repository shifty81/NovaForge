// Tests for: Space-Planet Transition, Fleet Hangar, Hangar Environment (Phase 14)
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_hangar_system.h"
#include "systems/hangar_environment_system.h"
#include "systems/space_planet_transition_system.h"
#include <sys/stat.h>

using namespace atlas;

// =====================================================
// Space-Planet Transition System Tests (Phase 14)

// =====================================================

static void testTransitionInit() {
    std::cout << "\n=== Space-Planet Transition: Init ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    assertTrue(sys.initializeTransition("ship_1", "planet_1", true), "Transition initialized");
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Starts in space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude starts at 1000");
    assertTrue(!sys.initializeTransition("ship_1", "planet_1", true), "Duplicate init fails");
}

static void testTransitionBeginDescent() {
    std::cout << "\n=== Space-Planet Transition: Begin Descent ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    assertTrue(sys.beginDescent("ship_1"), "Descent started");
    assertTrue(sys.getTransitionState("ship_1") == "orbit_entry", "State is orbit_entry");
    assertTrue(!sys.beginDescent("ship_1"), "Cannot start descent again while in transition");
}

static void testTransitionDescentSequence() {
    std::cout << "\n=== Space-Planet Transition: Descent Sequence ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    sys.beginDescent("ship_1");
    // Advance through OrbitEntry (10s), AtmosphereEntry (10s), DescentPhase (10s), LandingApproach (10s)
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    assertTrue(sys.getTransitionState("ship_1") == "landed", "Reached landed state");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 0.0f), "Altitude is 0 when landed");
}

static void testTransitionBeginLaunch() {
    std::cout << "\n=== Space-Planet Transition: Begin Launch ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    sys.beginDescent("ship_1");
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    assertTrue(sys.getTransitionState("ship_1") == "landed", "Is landed");
    assertTrue(sys.beginLaunch("ship_1"), "Launch started");
    assertTrue(sys.getTransitionState("ship_1") == "launch_sequence", "State is launch_sequence");
}

static void testTransitionLaunchSequence() {
    std::cout << "\n=== Space-Planet Transition: Launch Sequence ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    sys.beginDescent("ship_1");
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    sys.beginLaunch("ship_1");
    // Advance through LaunchSequence (10s), AtmosphereExit (10s), OrbitExit (10s)
    for (int i = 0; i < 30; i++) sys.update(1.0f);
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Returned to space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude reset to 1000");
}

static void testTransitionAbort() {
    std::cout << "\n=== Space-Planet Transition: Abort ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    sys.beginDescent("ship_1");
    sys.update(3.0f);  // Partially into orbit entry
    assertTrue(sys.abortTransition("ship_1"), "Abort succeeded");
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Returned to space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude reset");
}

static void testTransitionAbortInvalid() {
    std::cout << "\n=== Space-Planet Transition: Abort Invalid ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    assertTrue(!sys.abortTransition("ship_1"), "Cannot abort when InSpace");
    sys.beginDescent("ship_1");
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    assertTrue(!sys.abortTransition("ship_1"), "Cannot abort when Landed");
}

static void testTransitionAtmosphereHeat() {
    std::cout << "\n=== Space-Planet Transition: Atmosphere Heat ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    assertTrue(approxEqual(sys.getHeatLevel("ship_1"), 0.0f), "Initial heat is 0");
    sys.beginDescent("ship_1");
    // Advance through OrbitEntry (10s) into AtmosphereEntry
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    // Now in AtmosphereEntry, heat should build
    sys.update(5.0f);
    assertTrue(sys.getHeatLevel("ship_1") > 0.0f, "Heat built up during atmosphere entry");
}

static void testTransitionAutopilot() {
    std::cout << "\n=== Space-Planet Transition: Autopilot ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    assertTrue(sys.setAutopilot("ship_1", true), "Autopilot enabled");
    assertTrue(sys.setLandingTarget("ship_1", 100.0f, 200.0f), "Landing target set");
}

static void testTransitionMissing() {
    std::cout << "\n=== Space-Planet Transition: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    assertTrue(!sys.initializeTransition("nonexistent", "planet_1", true), "Init fails on missing");
    assertTrue(!sys.beginDescent("nonexistent"), "Descent fails on missing");
    assertTrue(approxEqual(sys.getAltitude("nonexistent"), 0.0f), "Altitude 0 on missing");
    assertTrue(sys.getTransitionState("nonexistent") == "unknown", "State unknown on missing");
}


// =====================================================
// Fleet Hangar System Tests (Phase 14)

// =====================================================

static void testFleetHangarInit() {
    std::cout << "\n=== Fleet Hangar: Init ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    assertTrue(sys.initializeHangar("station_1", "player1", "Main Hangar", 1), "Hangar initialized");
    assertTrue(sys.getTier("station_1") == 1, "Tier is 1");
    assertTrue(sys.getMaxSlots("station_1") == 5, "Max slots is 5 for tier 1");
    assertTrue(sys.getShipCount("station_1") == 0, "No ships docked");
    assertTrue(!sys.initializeHangar("station_1", "player1", "Dup", 1), "Duplicate init fails");
}

static void testFleetHangarDock() {
    std::cout << "\n=== Fleet Hangar: Dock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    assertTrue(sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Ship docked");
    assertTrue(sys.getShipCount("station_1") == 1, "Ship count is 1");
    assertTrue(!sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Duplicate dock fails");
}

static void testFleetHangarUndock() {
    std::cout << "\n=== Fleet Hangar: Undock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.dockShip("station_1", "ship_1", "frigate", 100.0f);
    assertTrue(sys.undockShip("station_1", "ship_1"), "Ship undocked");
    assertTrue(sys.getShipCount("station_1") == 0, "Ship count is 0");
    assertTrue(!sys.undockShip("station_1", "ship_1"), "Undock of absent ship fails");
}

static void testFleetHangarFull() {
    std::cout << "\n=== Fleet Hangar: Full ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    for (int i = 0; i < 5; i++) {
        sys.dockShip("station_1", "ship_" + std::to_string(i), "frigate", 100.0f);
    }
    assertTrue(sys.getShipCount("station_1") == 5, "Hangar full with 5 ships");
    assertTrue(!sys.dockShip("station_1", "ship_extra", "cruiser", 100.0f), "Cannot dock when full");
}

static void testFleetHangarLock() {
    std::cout << "\n=== Fleet Hangar: Lock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.dockShip("station_1", "ship_1", "frigate", 100.0f);
    assertTrue(sys.lockShip("station_1", "ship_1"), "Ship locked");
    assertTrue(!sys.undockShip("station_1", "ship_1"), "Cannot undock locked ship");
    assertTrue(sys.unlockShip("station_1", "ship_1"), "Ship unlocked");
    assertTrue(sys.undockShip("station_1", "ship_1"), "Undock after unlock");
}

static void testFleetHangarUpgrade() {
    std::cout << "\n=== Fleet Hangar: Upgrade ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    assertTrue(sys.upgradeHangar("station_1"), "Upgrade to tier 2");
    assertTrue(sys.getTier("station_1") == 2, "Tier is 2");
    assertTrue(sys.getMaxSlots("station_1") == 10, "Max slots is 10 for tier 2");
}

static void testFleetHangarMaxTier() {
    std::cout << "\n=== Fleet Hangar: Max Tier ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 5);
    assertTrue(sys.getTier("station_1") == 5, "Tier is 5");
    assertTrue(sys.getMaxSlots("station_1") == 50, "Max slots is 50 for tier 5");
    assertTrue(!sys.upgradeHangar("station_1"), "Cannot upgrade past tier 5");
}

static void testFleetHangarRepair() {
    std::cout << "\n=== Fleet Hangar: Repair ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.dockShip("station_1", "ship_1", "frigate", 50.0f);
    assertTrue(sys.repairShip("station_1", "ship_1", 30.0f), "Repair applied");
    // Verify ship count still correct
    assertTrue(sys.getShipCount("station_1") == 1, "Ship still docked after repair");
}

static void testFleetHangarPower() {
    std::cout << "\n=== Fleet Hangar: Power ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.setPowerEnabled("station_1", false);
    assertTrue(!sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Cannot dock without power");
    sys.setPowerEnabled("station_1", true);
    assertTrue(sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Dock with power");
    sys.setPowerEnabled("station_1", false);
    assertTrue(!sys.undockShip("station_1", "ship_1"), "Cannot undock without power");
}

static void testFleetHangarMissing() {
    std::cout << "\n=== Fleet Hangar: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    assertTrue(!sys.initializeHangar("nonexistent", "p", "H", 1), "Init fails on missing");
    assertTrue(!sys.dockShip("nonexistent", "s", "c", 100.0f), "Dock fails on missing");
    assertTrue(sys.getShipCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getTier("nonexistent") == 0, "Tier 0 on missing");
}


// =====================================================
// Hangar Environment System Tests (Phase 14)

// =====================================================

static void testHangarEnvInit() {
    std::cout << "\n=== Hangar Environment: Init ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    assertTrue(sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f), "Env initialized");
    assertTrue(approxEqual(sys.getToxicity("hangar_1"), 0.0f), "Initial toxicity is 0");
    assertTrue(!sys.isAlarmActive("hangar_1"), "Alarm not active initially");
    assertTrue(!sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f), "Duplicate init fails");
}

static void testHangarEnvOpenClose() {
    std::cout << "\n=== Hangar Environment: Open/Close ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Breathable, 22.0f, 1.0f);
    assertTrue(sys.openHangar("hangar_1"), "Hangar opened");
    assertTrue(sys.closeHangar("hangar_1"), "Hangar closed");
}

static void testHangarEnvToxicExposure() {
    std::cout << "\n=== Hangar Environment: Toxic Exposure ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -30.0f, 0.3f);
    sys.openHangar("hangar_1");
    // Update several ticks - toxicity should increase
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.getToxicity("hangar_1") > 0.0f, "Toxicity increased when open in toxic");
}

static void testHangarEnvCorrosiveExposure() {
    std::cout << "\n=== Hangar Environment: Corrosive Exposure ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Corrosive, 100.0f, 2.0f);
    sys.openHangar("hangar_1");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    // Check corrosion by getting occupant damage (need to add occupant)
    sys.addOccupant("hangar_1", "player_1", false, 0.0f);
    float damage = sys.getOccupantDamage("hangar_1", "player_1");
    assertTrue(damage > 0.0f, "Damage > 0 in corrosive atmosphere");
}

static void testHangarEnvUnsuitedDamage() {
    std::cout << "\n=== Hangar Environment: Unsuited Damage ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    sys.openHangar("hangar_1");
    sys.addOccupant("hangar_1", "player_1", false, 0.0f);
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    float damage = sys.getOccupantDamage("hangar_1", "player_1");
    assertTrue(damage > 0.0f, "Unsuited occupant takes damage");
}

static void testHangarEnvSuitedProtection() {
    std::cout << "\n=== Hangar Environment: Suited Protection ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    sys.openHangar("hangar_1");
    sys.addOccupant("hangar_1", "unsuited", false, 0.0f);
    sys.addOccupant("hangar_1", "suited", true, 0.8f);
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    float unsuited_dmg = sys.getOccupantDamage("hangar_1", "unsuited");
    float suited_dmg = sys.getOccupantDamage("hangar_1", "suited");
    assertTrue(suited_dmg < unsuited_dmg, "Suited occupant takes less damage");
}

static void testHangarEnvRecovery() {
    std::cout << "\n=== Hangar Environment: Recovery ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    sys.openHangar("hangar_1");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    float tox_open = sys.getToxicity("hangar_1");
    sys.closeHangar("hangar_1");
    for (int i = 0; i < 20; i++) sys.update(1.0f);
    float tox_closed = sys.getToxicity("hangar_1");
    assertTrue(tox_closed < tox_open, "Toxicity decreased after closing");
}

static void testHangarEnvAlarm() {
    std::cout << "\n=== Hangar Environment: Alarm ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    assertTrue(!sys.isAlarmActive("hangar_1"), "No alarm initially");
    sys.openHangar("hangar_1");
    // Run enough ticks for toxicity > 0.3 (mix rate 0.1/s, need > 3 seconds)
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.isAlarmActive("hangar_1"), "Alarm active when toxicity > 0.3");
}

static void testHangarEnvVacuum() {
    std::cout << "\n=== Hangar Environment: Vacuum ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::None, -270.0f, 0.0f);
    sys.openHangar("hangar_1");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    // In vacuum, internal pressure should drop
    auto* entity = world.getEntity("hangar_1");
    auto* env = entity->getComponent<components::HangarEnvironment>();
    assertTrue(env->internal_pressure < 1.0f, "Pressure dropped in vacuum");
}

static void testHangarEnvMissing() {
    std::cout << "\n=== Hangar Environment: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    assertTrue(!sys.initializeEnvironment("nonexistent",
        components::HangarEnvironment::AtmosphereType::Toxic, 0.0f, 0.0f), "Init fails on missing");
    assertTrue(!sys.openHangar("nonexistent"), "Open fails on missing");
    assertTrue(approxEqual(sys.getToxicity("nonexistent"), 0.0f), "Toxicity 0 on missing");
    assertTrue(sys.getOccupantCount("nonexistent") == 0, "Count 0 on missing");
}


void run_misc_tests() {
    testTransitionInit();
    testTransitionBeginDescent();
    testTransitionDescentSequence();
    testTransitionBeginLaunch();
    testTransitionLaunchSequence();
    testTransitionAbort();
    testTransitionAbortInvalid();
    testTransitionAtmosphereHeat();
    testTransitionAutopilot();
    testTransitionMissing();
    testFleetHangarInit();
    testFleetHangarDock();
    testFleetHangarUndock();
    testFleetHangarFull();
    testFleetHangarLock();
    testFleetHangarUpgrade();
    testFleetHangarMaxTier();
    testFleetHangarRepair();
    testFleetHangarPower();
    testFleetHangarMissing();
    testHangarEnvInit();
    testHangarEnvOpenClose();
    testHangarEnvToxicExposure();
    testHangarEnvCorrosiveExposure();
    testHangarEnvUnsuitedDamage();
    testHangarEnvSuitedProtection();
    testHangarEnvRecovery();
    testHangarEnvAlarm();
    testHangarEnvVacuum();
    testHangarEnvMissing();
}
