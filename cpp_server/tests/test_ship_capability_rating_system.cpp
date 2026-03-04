// Tests for: ShipCapabilityRating System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ship_capability_rating_system.h"

using namespace atlas;

// ==================== ShipCapabilityRating System Tests ====================

static void testShipRatingCreate() {
    std::cout << "\n=== ShipRating: Create ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeRating("ship1"), "Init rating succeeds");
    assertTrue(approxEqual(sys.getOverallRating("ship1"), 0.0f), "Overall rating starts at 0");
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 0.0f), "Combat rating starts at 0");
    assertTrue(approxEqual(sys.getMiningRating("ship1"), 0.0f), "Mining rating starts at 0");
}

static void testShipRatingCombat() {
    std::cout << "\n=== ShipRating: Combat ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setWeaponCount("ship1", 8);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 5.0f), "8 weapons = 5 star combat");
    sys.setWeaponCount("ship1", 4);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 2.5f), "4 weapons = 2.5 star combat");
}

static void testShipRatingMining() {
    std::cout << "\n=== ShipRating: Mining ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setMiningModuleCount("ship1", 5);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getMiningRating("ship1"), 5.0f), "5 mining modules = 5 stars");
    sys.setMiningModuleCount("ship1", 0);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getMiningRating("ship1"), 0.0f), "0 mining modules = 0 stars");
}

static void testShipRatingExploration() {
    std::cout << "\n=== ShipRating: Exploration ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setScannerCount("ship1", 4);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getExplorationRating("ship1"), 5.0f), "4 scanners = 5 star exploration");
    sys.setScannerCount("ship1", 2);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getExplorationRating("ship1"), 2.5f), "2 scanners = 2.5 stars");
}

static void testShipRatingCargo() {
    std::cout << "\n=== ShipRating: Cargo ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setCargoCapacity("ship1", 50000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCargoRating("ship1"), 5.0f), "50000 m3 = 5 star cargo");
    sys.setCargoCapacity("ship1", 25000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCargoRating("ship1"), 2.5f), "25000 m3 = 2.5 star cargo");
}

static void testShipRatingDefense() {
    std::cout << "\n=== ShipRating: Defense ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setTotalEHP("ship1", 100000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getDefenseRating("ship1"), 5.0f), "100k EHP = 5 star defense");
    sys.setTotalEHP("ship1", 50000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getDefenseRating("ship1"), 2.5f), "50k EHP = 2.5 star defense");
}

static void testShipRatingFabrication() {
    std::cout << "\n=== ShipRating: Fabrication ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setIndustryModuleCount("ship1", 5);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getFabricationRating("ship1"), 5.0f), "5 industry = 5 star fabrication");
}

static void testShipRatingOverall() {
    std::cout << "\n=== ShipRating: Overall ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setWeaponCount("ship1", 8);
    sys.setMiningModuleCount("ship1", 5);
    sys.setScannerCount("ship1", 4);
    sys.setCargoCapacity("ship1", 50000.0f);
    sys.setTotalEHP("ship1", 100000.0f);
    sys.setIndustryModuleCount("ship1", 5);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getOverallRating("ship1"), 5.0f), "All maxed = 5 star overall");
}

static void testShipRatingRecalculate() {
    std::cout << "\n=== ShipRating: Recalculate ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setWeaponCount("ship1", 4);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 2.5f), "Initial combat 2.5");
    // After update, needs_recalculation is false; modify and force recalc
    assertTrue(sys.recalculate("ship1"), "Recalculate succeeds");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 2.5f), "Recalculated same value");
}

static void testShipRatingMissing() {
    std::cout << "\n=== ShipRating: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    assertTrue(!sys.initializeRating("nonexistent"), "Init on missing entity fails");
    assertTrue(approxEqual(sys.getCombatRating("nonexistent"), 0.0f), "Combat rating 0 on missing");
    assertTrue(approxEqual(sys.getOverallRating("nonexistent"), 0.0f), "Overall rating 0 on missing");
    assertTrue(!sys.setWeaponCount("nonexistent", 5), "Set weapons on missing fails");
}


void run_ship_capability_rating_system_tests() {
    testShipRatingCreate();
    testShipRatingCombat();
    testShipRatingMining();
    testShipRatingExploration();
    testShipRatingCargo();
    testShipRatingDefense();
    testShipRatingFabrication();
    testShipRatingOverall();
    testShipRatingRecalculate();
    testShipRatingMissing();
}
