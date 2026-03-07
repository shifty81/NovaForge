// Tests for: Asteroid Respawn System
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/asteroid_respawn_system.h"

using namespace atlas;

// ==================== Asteroid Respawn System Tests ====================

static void testAsteroidRespawnCreate() {
    std::cout << "\n=== AsteroidRespawn: Create ===" << std::endl;
    ecs::World world;
    systems::AsteroidRespawnSystem sys(&world);
    world.createEntity("belt1");
    assertTrue(sys.initialize("belt1", "belt_alpha", "system_01", 20), "Init succeeds");
    assertTrue(sys.getState("belt1") == "Full", "Full initially");
    assertTrue(sys.getTotalAsteroids("belt1") == 20, "20 asteroids");
    assertTrue(sys.getMaxAsteroids("belt1") == 20, "Max 20");
    assertTrue(sys.getDepletedCount("belt1") == 0, "None depleted");
    assertTrue(approxEqual(sys.getDepletionPct("belt1"), 0.0f), "0% depleted");
    assertTrue(sys.getTotalRespawned("belt1") == 0, "0 respawned");
    assertTrue(sys.getTotalDepleted("belt1") == 0, "0 total depleted");
}

static void testAsteroidRespawnDepletion() {
    std::cout << "\n=== AsteroidRespawn: Depletion ===" << std::endl;
    ecs::World world;
    systems::AsteroidRespawnSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", "belt_alpha", "system_01", 20);

    assertTrue(sys.deplete("belt1", 5), "Deplete 5");
    sys.update(0.1f);  // tick to update percentages
    assertTrue(sys.getTotalAsteroids("belt1") == 15, "15 remaining");
    assertTrue(sys.getDepletedCount("belt1") == 5, "5 depleted");
    assertTrue(sys.getTotalDepleted("belt1") == 5, "5 total depleted");
    assertTrue(sys.getDepletionPct("belt1") > 0.0f, "Depletion > 0%");

    // Cannot deplete more than available
    assertTrue(sys.deplete("belt1", 100), "Deplete max");
    sys.update(0.1f);
    assertTrue(sys.getTotalAsteroids("belt1") == 0, "0 remaining");
    assertTrue(sys.getState("belt1") == "Depleted", "Depleted state");
}

static void testAsteroidRespawnRegeneration() {
    std::cout << "\n=== AsteroidRespawn: Regeneration ===" << std::endl;
    ecs::World world;
    systems::AsteroidRespawnSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", "belt_alpha", "system_01", 10);
    sys.setRespawnRate("belt1", 1.0f);  // 1 asteroid per second
    sys.setRegenerationDelay("belt1", 5.0f);  // 5 second delay

    sys.deplete("belt1", 5);
    assertTrue(sys.getTotalAsteroids("belt1") == 5, "5 remaining after depletion");

    // During delay period, no respawning
    sys.update(3.0f);
    assertTrue(sys.getTotalAsteroids("belt1") == 5, "Still 5 during delay");

    // After delay, respawning begins
    sys.update(3.0f);  // 3 more seconds = past 5s delay, ~1s of respawn
    assertTrue(sys.getTotalAsteroids("belt1") > 5, "Respawning started");
    assertTrue(sys.getTotalRespawned("belt1") > 0, "Some respawned");
}

static void testAsteroidRespawnFullRegeneration() {
    std::cout << "\n=== AsteroidRespawn: FullRegeneration ===" << std::endl;
    ecs::World world;
    systems::AsteroidRespawnSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", "belt_alpha", "system_01", 5);
    sys.setRespawnRate("belt1", 10.0f);  // fast respawn
    sys.setRegenerationDelay("belt1", 0.0f);  // no delay

    sys.deplete("belt1", 5);
    assertTrue(sys.getTotalAsteroids("belt1") == 0, "Fully depleted");

    // Fast respawn should regenerate all
    sys.update(10.0f);
    assertTrue(sys.getTotalAsteroids("belt1") == 5, "Fully regenerated");
    assertTrue(sys.getState("belt1") == "Full", "Full state");
    assertTrue(sys.getTotalRespawned("belt1") == 5, "5 respawned");
}

static void testAsteroidRespawnRateConfig() {
    std::cout << "\n=== AsteroidRespawn: RateConfig ===" << std::endl;
    ecs::World world;
    systems::AsteroidRespawnSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", "belt_alpha", "system_01", 20);

    assertTrue(sys.setRespawnRate("belt1", 0.5f), "Set respawn rate");
    assertTrue(sys.setRegenerationDelay("belt1", 30.0f), "Set regen delay");
    assertTrue(!sys.deplete("belt1", 0), "Cannot deplete 0");
    assertTrue(!sys.deplete("belt1", -1), "Cannot deplete negative");
}

static void testAsteroidRespawnMissing() {
    std::cout << "\n=== AsteroidRespawn: Missing ===" << std::endl;
    ecs::World world;
    systems::AsteroidRespawnSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "belt", "sys", 10), "Init fails on missing");
    assertTrue(!sys.deplete("nonexistent", 1), "Deplete fails on missing");
    assertTrue(!sys.setRespawnRate("nonexistent", 1.0f), "SetRate fails on missing");
    assertTrue(!sys.setRegenerationDelay("nonexistent", 1.0f), "SetDelay fails on missing");
    assertTrue(sys.getState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(sys.getTotalAsteroids("nonexistent") == 0, "0 asteroids on missing");
    assertTrue(sys.getMaxAsteroids("nonexistent") == 0, "0 max on missing");
    assertTrue(sys.getDepletedCount("nonexistent") == 0, "0 depleted on missing");
    assertTrue(approxEqual(sys.getDepletionPct("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getTotalRespawned("nonexistent") == 0, "0 respawned on missing");
    assertTrue(sys.getTotalDepleted("nonexistent") == 0, "0 total depleted on missing");
}

void run_asteroid_respawn_system_tests() {
    testAsteroidRespawnCreate();
    testAsteroidRespawnDepletion();
    testAsteroidRespawnRegeneration();
    testAsteroidRespawnFullRegeneration();
    testAsteroidRespawnRateConfig();
    testAsteroidRespawnMissing();
}
