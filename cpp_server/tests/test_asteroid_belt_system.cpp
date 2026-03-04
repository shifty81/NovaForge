// Tests for: AsteroidBelt System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/asteroid_belt_system.h"

using namespace atlas;

// ==================== AsteroidBelt System Tests ====================

static void testAsteroidBeltCreate() {
    std::cout << "\n=== AsteroidBelt: Create ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    assertTrue(sys.initializeBelt("belt1", "belt_alpha", "sys1"), "Init belt succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 0, "No asteroids initially");
    assertTrue(sys.getTotalMined("belt1") == 0, "No mined initially");
    assertTrue(sys.getTotalRespawned("belt1") == 0, "No respawned initially");
}

static void testAsteroidBeltAdd() {
    std::cout << "\n=== AsteroidBelt: Add ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    assertTrue(sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f), "Add asteroid succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "1 asteroid");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 5000.0f), "5000 ore remaining");
}

static void testAsteroidBeltDuplicate() {
    std::cout << "\n=== AsteroidBelt: Duplicate ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f);
    assertTrue(!sys.addAsteroid("belt1", "ast1", "Scordite", 3000.0f, 1.5f), "Duplicate rejected");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "Still 1 asteroid");
}

static void testAsteroidBeltMine() {
    std::cout << "\n=== AsteroidBelt: Mine ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f);
    float mined = sys.mineAsteroid("belt1", "ast1", 1000.0f);
    assertTrue(approxEqual(mined, 1000.0f), "Mined 1000 ore");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 4000.0f), "4000 remaining");
    assertTrue(!sys.isAsteroidDepleted("belt1", "ast1"), "Not depleted");
}

static void testAsteroidBeltDeplete() {
    std::cout << "\n=== AsteroidBelt: Deplete ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Scordite", 1000.0f, 1.0f);
    sys.mineAsteroid("belt1", "ast1", 1000.0f);
    assertTrue(sys.isAsteroidDepleted("belt1", "ast1"), "Asteroid depleted");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 0.0f), "0 ore remaining");
    assertTrue(sys.getTotalMined("belt1") == 1, "1 total mined");
    assertTrue(sys.getDepletedCount("belt1") == 1, "1 depleted");
}

static void testAsteroidBeltRichness() {
    std::cout << "\n=== AsteroidBelt: Richness ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Kernite", 5000.0f, 2.0f);
    // Mining 500 with richness 2.0 → actual = 500 * 2.0 = 1000
    float mined = sys.mineAsteroid("belt1", "ast1", 500.0f);
    assertTrue(approxEqual(mined, 1000.0f), "Richness doubles yield");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 4000.0f), "4000 remaining");
}

static void testAsteroidBeltRespawn() {
    std::cout << "\n=== AsteroidBelt: Respawn ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f);
    sys.mineAsteroid("belt1", "ast1", 1000.0f); // deplete
    assertTrue(sys.isAsteroidDepleted("belt1", "ast1"), "Depleted after mining");
    // Advance past respawn interval (default 3600s)
    sys.update(3600.0f);
    assertTrue(!sys.isAsteroidDepleted("belt1", "ast1"), "Respawned after interval");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 1000.0f), "Full ore restored");
    assertTrue(sys.getTotalRespawned("belt1") == 1, "1 respawn counted");
}

static void testAsteroidBeltRemove() {
    std::cout << "\n=== AsteroidBelt: Remove ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f);
    sys.addAsteroid("belt1", "ast2", "Scordite", 3000.0f, 1.0f);
    assertTrue(sys.removeAsteroid("belt1", "ast1"), "Remove succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "1 asteroid remaining");
    assertTrue(!sys.removeAsteroid("belt1", "ast1"), "Remove nonexistent fails");
}

static void testAsteroidBeltMaxLimit() {
    std::cout << "\n=== AsteroidBelt: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    auto* entity = world.getEntity("belt1");
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    belt->max_asteroids = 3;
    sys.addAsteroid("belt1", "a1", "Veldspar", 1000.0f, 1.0f);
    sys.addAsteroid("belt1", "a2", "Scordite", 1000.0f, 1.0f);
    sys.addAsteroid("belt1", "a3", "Pyroxeres", 1000.0f, 1.0f);
    assertTrue(!sys.addAsteroid("belt1", "a4", "Kernite", 1000.0f, 1.0f), "Max limit enforced");
    assertTrue(sys.getAsteroidCount("belt1") == 3, "Still 3 asteroids");
}

static void testAsteroidBeltMissing() {
    std::cout << "\n=== AsteroidBelt: Missing ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    assertTrue(!sys.initializeBelt("nonexistent", "b1", "s1"), "Init fails on missing");
    assertTrue(!sys.addAsteroid("nonexistent", "a1", "Ore", 100.0f, 1.0f), "Add fails on missing");
    assertTrue(!sys.removeAsteroid("nonexistent", "a1"), "Remove fails on missing");
    assertTrue(approxEqual(sys.mineAsteroid("nonexistent", "a1", 100.0f), 0.0f), "Mine returns 0 on missing");
    assertTrue(sys.getAsteroidCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getDepletedCount("nonexistent") == 0, "0 depleted on missing");
    assertTrue(approxEqual(sys.getRemainingOre("nonexistent", "a1"), 0.0f), "0 ore on missing");
    assertTrue(!sys.isAsteroidDepleted("nonexistent", "a1"), "Not depleted on missing");
    assertTrue(sys.getTotalMined("nonexistent") == 0, "0 mined on missing");
    assertTrue(sys.getTotalRespawned("nonexistent") == 0, "0 respawned on missing");
}


void run_asteroid_belt_system_tests() {
    testAsteroidBeltCreate();
    testAsteroidBeltAdd();
    testAsteroidBeltDuplicate();
    testAsteroidBeltMine();
    testAsteroidBeltDeplete();
    testAsteroidBeltRichness();
    testAsteroidBeltRespawn();
    testAsteroidBeltRemove();
    testAsteroidBeltMaxLimit();
    testAsteroidBeltMissing();
}
