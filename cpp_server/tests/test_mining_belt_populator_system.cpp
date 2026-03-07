// Tests for: Mining Belt Populator System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/mining_belt_populator_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mining Belt Populator System Tests ====================

static void testMiningBeltPopulatorCreate() {
    std::cout << "\n=== MiningBeltPopulator: Create ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    assertTrue(sys.initialize("belt1"), "Init succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 0, "No asteroids initially");
    assertTrue(sys.getDepletedCount("belt1") == 0, "No depleted initially");
    assertTrue(approxEqual(sys.getTotalOreExtracted("belt1"), 0.0f), "No ore extracted");
    assertTrue(sys.getTotalMined("belt1") == 0, "No mined counter");
    assertTrue(sys.getTotalRespawned("belt1") == 0, "No respawned counter");
}

static void testMiningBeltPopulatorAddAsteroids() {
    std::cout << "\n=== MiningBeltPopulator: AddAsteroids ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");
    assertTrue(sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f), "Add ast1");
    assertTrue(sys.addAsteroid("belt1", "ast2", "Scordite", 2000.0f, 1.5f), "Add ast2");
    assertTrue(sys.addAsteroid("belt1", "ast3", "Pyroxeres", 500.0f, 0.8f), "Add ast3");
    assertTrue(sys.getAsteroidCount("belt1") == 3, "3 asteroids added");
}

static void testMiningBeltPopulatorDuplicate() {
    std::cout << "\n=== MiningBeltPopulator: Duplicate ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f);
    assertTrue(!sys.addAsteroid("belt1", "ast1", "Scordite", 500.0f, 1.0f), "Duplicate rejected");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "Still 1 asteroid");
}

static void testMiningBeltPopulatorExtractOre() {
    std::cout << "\n=== MiningBeltPopulator: ExtractOre ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f);
    assertTrue(sys.extractOre("belt1", "ast1", 300.0f), "Extract 300 succeeds");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 700.0f), "700 remaining");
    assertTrue(approxEqual(sys.getTotalOreExtracted("belt1"), 300.0f), "Total extracted 300");
    assertTrue(!sys.extractOre("belt1", "nonexistent", 100.0f), "Extract from missing fails");
}

static void testMiningBeltPopulatorDepletion() {
    std::cout << "\n=== MiningBeltPopulator: Depletion ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 100.0f, 1.0f);
    sys.extractOre("belt1", "ast1", 100.0f);
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 0.0f), "0 remaining after full extract");

    sys.update(0.1f); // tick to mark depleted
    assertTrue(sys.isAsteroidDepleted("belt1", "ast1"), "Asteroid is depleted");
    assertTrue(sys.getDepletedCount("belt1") == 1, "1 depleted");
    assertTrue(sys.getTotalMined("belt1") == 1, "1 total mined");
    assertTrue(!sys.extractOre("belt1", "ast1", 50.0f), "Can't extract from depleted");
}

static void testMiningBeltPopulatorRespawn() {
    std::cout << "\n=== MiningBeltPopulator: Respawn ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");

    auto* entity = world.getEntity("belt1");
    auto* comp = entity->getComponent<components::MiningBeltPopulator>();
    comp->respawn_interval = 10.0f; // short respawn for test

    sys.addAsteroid("belt1", "ast1", "Veldspar", 100.0f, 1.0f);
    sys.extractOre("belt1", "ast1", 100.0f);
    sys.update(0.1f); // mark depleted
    assertTrue(sys.isAsteroidDepleted("belt1", "ast1"), "Depleted before respawn");

    sys.update(10.0f); // trigger respawn
    assertTrue(!sys.isAsteroidDepleted("belt1", "ast1"), "No longer depleted after respawn");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 100.0f), "Ore restored to initial");
    assertTrue(sys.getTotalRespawned("belt1") == 1, "1 total respawned");
}

static void testMiningBeltPopulatorRemove() {
    std::cout << "\n=== MiningBeltPopulator: Remove ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f);
    assertTrue(sys.removeAsteroid("belt1", "ast1"), "Remove succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 0, "0 asteroids after remove");
    assertTrue(!sys.removeAsteroid("belt1", "ast1"), "Double remove fails");
}

static void testMiningBeltPopulatorMaxLimit() {
    std::cout << "\n=== MiningBeltPopulator: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");

    auto* entity = world.getEntity("belt1");
    auto* comp = entity->getComponent<components::MiningBeltPopulator>();
    comp->max_asteroids = 2;

    sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f);
    sys.addAsteroid("belt1", "ast2", "Scordite", 2000.0f, 1.5f);
    assertTrue(!sys.addAsteroid("belt1", "ast3", "Pyroxeres", 500.0f, 0.8f), "Max limit enforced");
    assertTrue(sys.getAsteroidCount("belt1") == 2, "Still 2 asteroids");
}

static void testMiningBeltPopulatorOverExtract() {
    std::cout << "\n=== MiningBeltPopulator: OverExtract ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 100.0f, 1.0f);
    assertTrue(sys.extractOre("belt1", "ast1", 200.0f), "Over-extract clamped");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 0.0f), "Remaining is 0");
    assertTrue(approxEqual(sys.getTotalOreExtracted("belt1"), 100.0f), "Only 100 actually extracted");
}

static void testMiningBeltPopulatorMissing() {
    std::cout << "\n=== MiningBeltPopulator: Missing ===" << std::endl;
    ecs::World world;
    systems::MiningBeltPopulatorSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addAsteroid("nonexistent", "a1", "Veldspar", 100.0f, 1.0f), "Add fails on missing");
    assertTrue(!sys.removeAsteroid("nonexistent", "a1"), "Remove fails on missing");
    assertTrue(!sys.extractOre("nonexistent", "a1", 50.0f), "Extract fails on missing");
    assertTrue(sys.getAsteroidCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getDepletedCount("nonexistent") == 0, "0 depleted on missing");
    assertTrue(approxEqual(sys.getRemainingOre("nonexistent", "a1"), 0.0f), "0 remaining on missing");
    assertTrue(approxEqual(sys.getTotalOreExtracted("nonexistent"), 0.0f), "0 extracted on missing");
    assertTrue(sys.getTotalMined("nonexistent") == 0, "0 mined on missing");
    assertTrue(sys.getTotalRespawned("nonexistent") == 0, "0 respawned on missing");
    assertTrue(!sys.isAsteroidDepleted("nonexistent", "a1"), "Not depleted on missing");
}


void run_mining_belt_populator_system_tests() {
    testMiningBeltPopulatorCreate();
    testMiningBeltPopulatorAddAsteroids();
    testMiningBeltPopulatorDuplicate();
    testMiningBeltPopulatorExtractOre();
    testMiningBeltPopulatorDepletion();
    testMiningBeltPopulatorRespawn();
    testMiningBeltPopulatorRemove();
    testMiningBeltPopulatorMaxLimit();
    testMiningBeltPopulatorOverExtract();
    testMiningBeltPopulatorMissing();
}
