// Tests for: Star System Populator System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/star_system_populator_system.h"

using namespace atlas;

// ==================== Star System Populator System Tests ====================

static void testPopulatorCreate() {
    std::cout << "\n=== Populator: Create ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    assertTrue(sys.initialize("sys1", "sol_001", "Sol", 0.9f), "Init succeeds");
    assertTrue(sys.getStationCount("sys1") == 0, "No stations");
    assertTrue(sys.getAsteroidBeltCount("sys1") == 0, "No asteroid belts");
    assertTrue(sys.getNPCFactionCount("sys1") == 0, "No NPC factions");
    assertTrue(sys.getJumpGateCount("sys1") == 0, "No jump gates");
    assertTrue(sys.getPointOfInterestCount("sys1") == 0, "No POIs");
    assertTrue(!sys.isPopulated("sys1"), "Not populated");
    assertTrue(approxEqual(sys.getPopulationTime("sys1"), 0.0f), "No population time");
}

static void testPopulatorStations() {
    std::cout << "\n=== Populator: Stations ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    assertTrue(sys.addStation("sys1", "sta_1", "Jita 4-4", "Trade"), "Add station 1");
    assertTrue(sys.addStation("sys1", "sta_2", "Amarr Hub", "Industrial"), "Add station 2");
    assertTrue(sys.getStationCount("sys1") == 2, "2 stations");
    assertTrue(sys.hasStation("sys1", "sta_1"), "Has station 1");
    assertTrue(sys.hasStation("sys1", "sta_2"), "Has station 2");
    assertTrue(!sys.hasStation("sys1", "sta_3"), "No station 3");
    assertTrue(!sys.addStation("sys1", "sta_1", "Dup", "Trade"), "Duplicate rejected");
}

static void testPopulatorStationMax() {
    std::cout << "\n=== Populator: StationMax ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::StarSystemPopulation>();
    state->max_stations = 2;
    sys.addStation("sys1", "s1", "A", "Trade");
    sys.addStation("sys1", "s2", "B", "Industrial");
    assertTrue(!sys.addStation("sys1", "s3", "C", "Military"), "Max stations enforced");
}

static void testPopulatorAsteroidBelts() {
    std::cout << "\n=== Populator: AsteroidBelts ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    assertTrue(sys.addAsteroidBelt("sys1", "belt_1", "Veldspar", 3), "Add belt 1");
    assertTrue(sys.addAsteroidBelt("sys1", "belt_2", "Scordite", 5), "Add belt 2");
    assertTrue(sys.getAsteroidBeltCount("sys1") == 2, "2 belts");
    assertTrue(sys.hasAsteroidBelt("sys1", "belt_1"), "Has belt 1");
    assertTrue(!sys.hasAsteroidBelt("sys1", "belt_3"), "No belt 3");
    assertTrue(!sys.addAsteroidBelt("sys1", "belt_1", "Dup", 1), "Duplicate rejected");
}

static void testPopulatorNPCFactions() {
    std::cout << "\n=== Populator: NPCFactions ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    assertTrue(sys.addNPCFaction("sys1", "fac_1", "Serpentis", 5), "Add faction 1");
    assertTrue(sys.addNPCFaction("sys1", "fac_2", "Guristas", 3), "Add faction 2");
    assertTrue(sys.getNPCFactionCount("sys1") == 2, "2 factions");
    assertTrue(sys.hasNPCFaction("sys1", "fac_1"), "Has faction 1");
    assertTrue(!sys.hasNPCFaction("sys1", "fac_3"), "No faction 3");
    assertTrue(!sys.addNPCFaction("sys1", "fac_1", "Dup", 1), "Duplicate rejected");
}

static void testPopulatorJumpGates() {
    std::cout << "\n=== Populator: JumpGates ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    assertTrue(sys.addJumpGate("sys1", "gate_1", "Amarr"), "Add gate 1");
    assertTrue(sys.addJumpGate("sys1", "gate_2", "Caldari"), "Add gate 2");
    assertTrue(sys.getJumpGateCount("sys1") == 2, "2 gates");
    assertTrue(sys.hasJumpGate("sys1", "gate_1"), "Has gate 1");
    assertTrue(!sys.hasJumpGate("sys1", "gate_3"), "No gate 3");
    assertTrue(!sys.addJumpGate("sys1", "gate_1", "Dup"), "Duplicate rejected");
}

static void testPopulatorPOIs() {
    std::cout << "\n=== Populator: POIs ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    assertTrue(sys.addPointOfInterest("sys1", "poi_1", "Anomaly", "Pirate den"), "Add POI 1");
    assertTrue(sys.addPointOfInterest("sys1", "poi_2", "Wreck", "Abandoned ship"), "Add POI 2");
    assertTrue(sys.getPointOfInterestCount("sys1") == 2, "2 POIs");
    assertTrue(sys.hasPointOfInterest("sys1", "poi_1"), "Has POI 1");
    assertTrue(!sys.hasPointOfInterest("sys1", "poi_3"), "No POI 3");
    assertTrue(!sys.addPointOfInterest("sys1", "poi_1", "Dup", "Dup"), "Duplicate rejected");
}

static void testPopulatorMarkPopulated() {
    std::cout << "\n=== Populator: MarkPopulated ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    sys.addStation("sys1", "s1", "Hub", "Trade");
    sys.addAsteroidBelt("sys1", "b1", "Veldspar", 3);
    assertTrue(!sys.isPopulated("sys1"), "Not populated yet");
    assertTrue(sys.markPopulated("sys1", 10.0f), "Mark populated");
    assertTrue(sys.isPopulated("sys1"), "Is populated");
    assertTrue(approxEqual(sys.getPopulationTime("sys1"), 10.0f), "Population time 10s");
    assertTrue(!sys.markPopulated("sys1", 20.0f), "Double mark fails");
}

static void testPopulatorUpdate() {
    std::cout << "\n=== Populator: Update ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "sol_001", "Sol", 0.9f);
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::StarSystemPopulation>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testPopulatorMissing() {
    std::cout << "\n=== Populator: Missing ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulatorSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x", "x", 0.5f), "Init fails on missing");
    assertTrue(!sys.addStation("nonexistent", "s", "n", "t"), "addStation fails");
    assertTrue(sys.getStationCount("nonexistent") == 0, "0 stations");
    assertTrue(!sys.hasStation("nonexistent", "s"), "hasStation false");
    assertTrue(!sys.addAsteroidBelt("nonexistent", "b", "o", 1), "addAsteroidBelt fails");
    assertTrue(sys.getAsteroidBeltCount("nonexistent") == 0, "0 belts");
    assertTrue(!sys.addNPCFaction("nonexistent", "f", "n", 1), "addNPCFaction fails");
    assertTrue(sys.getNPCFactionCount("nonexistent") == 0, "0 factions");
    assertTrue(!sys.addJumpGate("nonexistent", "g", "d"), "addJumpGate fails");
    assertTrue(sys.getJumpGateCount("nonexistent") == 0, "0 gates");
    assertTrue(!sys.addPointOfInterest("nonexistent", "p", "t", "d"), "addPOI fails");
    assertTrue(sys.getPointOfInterestCount("nonexistent") == 0, "0 POIs");
    assertTrue(!sys.isPopulated("nonexistent"), "not populated");
    assertTrue(!sys.markPopulated("nonexistent", 0), "markPopulated fails");
    assertTrue(approxEqual(sys.getPopulationTime("nonexistent"), 0.0f), "0 population time");
}

void run_star_system_populator_system_tests() {
    testPopulatorCreate();
    testPopulatorStations();
    testPopulatorStationMax();
    testPopulatorAsteroidBelts();
    testPopulatorNPCFactions();
    testPopulatorJumpGates();
    testPopulatorPOIs();
    testPopulatorMarkPopulated();
    testPopulatorUpdate();
    testPopulatorMissing();
}
