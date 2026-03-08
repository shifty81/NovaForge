// Tests for: Asteroid Depletion Tracker System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/asteroid_depletion_tracker_system.h"


using namespace atlas;

// ==================== Asteroid Depletion Tracker System Tests ====================

static void testAsteroidDepletionTrackerCreate() {
    std::cout << "\n=== AsteroidDepletionTracker: Create ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    assertTrue(sys.initialize("adt1"), "Init succeeds");
    assertTrue(approxEqual(sys.getRemainingOre("adt1"), 10000.0f), "Default remaining ore");
    assertTrue(approxEqual(sys.getTotalVolume("adt1"), 10000.0f), "Default total volume");
    assertTrue(approxEqual(sys.getDepletionPercent("adt1"), 0.0f), "0% depletion initially");
    assertTrue(!sys.isDepleted("adt1"), "Not depleted initially");
    assertTrue(sys.getTimesDepleted("adt1") == 0, "0 depletions initially");
}

static void testAsteroidDepletionTrackerCustomVolume() {
    std::cout << "\n=== AsteroidDepletionTracker: CustomVolume ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    assertTrue(sys.initialize("adt1", 5000.0f), "Init with custom volume");
    assertTrue(approxEqual(sys.getTotalVolume("adt1"), 5000.0f), "Volume is 5000");
    assertTrue(approxEqual(sys.getRemainingOre("adt1"), 5000.0f), "Remaining is 5000");
    world.createEntity("adt2");
    assertTrue(!sys.initialize("adt2", -100.0f), "Negative volume rejected");
}

static void testAsteroidDepletionTrackerExtract() {
    std::cout << "\n=== AsteroidDepletionTracker: Extract ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    sys.initialize("adt1", 1000.0f);
    assertTrue(sys.extractOre("adt1", 400.0f), "Extract 400");
    assertTrue(approxEqual(sys.getRemainingOre("adt1"), 600.0f), "600 remaining");
    float pct = sys.getDepletionPercent("adt1");
    assertTrue(pct > 0.39f && pct < 0.41f, "40% depleted");
    assertTrue(!sys.extractOre("adt1", -10.0f), "Negative extract rejected");
}

static void testAsteroidDepletionTrackerFullDepletion() {
    std::cout << "\n=== AsteroidDepletionTracker: FullDepletion ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    sys.initialize("adt1", 100.0f);
    assertTrue(sys.extractOre("adt1", 100.0f), "Extract all ore");
    assertTrue(sys.isDepleted("adt1"), "Now depleted");
    assertTrue(approxEqual(sys.getRemainingOre("adt1"), 0.0f), "0 remaining");
    assertTrue(sys.getTimesDepleted("adt1") == 1, "1 depletion");
    assertTrue(!sys.extractOre("adt1", 10.0f), "Cannot extract from depleted");
}

static void testAsteroidDepletionTrackerSecurityBonus() {
    std::cout << "\n=== AsteroidDepletionTracker: SecurityBonus ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    sys.initialize("adt1");
    assertTrue(sys.setSecurityBonus("adt1", 1.5f), "Set security bonus");
    assertTrue(approxEqual(sys.getSecurityBonus("adt1"), 1.5f), "Bonus is 1.5");
    sys.setSecurityBonus("adt1", 5.0f);  // clamp to 2.0
    assertTrue(approxEqual(sys.getSecurityBonus("adt1"), 2.0f), "Clamped to 2.0");
    sys.setSecurityBonus("adt1", 0.1f);  // clamp to 0.5
    assertTrue(approxEqual(sys.getSecurityBonus("adt1"), 0.5f), "Clamped to 0.5");
}

static void testAsteroidDepletionTrackerRespawn() {
    std::cout << "\n=== AsteroidDepletionTracker: Respawn ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    sys.initialize("adt1", 100.0f);
    sys.setRespawnRate("adt1", 50.0f);   // 50 ore/s
    sys.setRespawnDelay("adt1", 10.0f);  // 10s delay
    sys.extractOre("adt1", 100.0f);      // deplete
    assertTrue(sys.isDepleted("adt1"), "Depleted");
    // Advance 5s - still in delay
    sys.update(5.0f);
    assertTrue(sys.isDepleted("adt1"), "Still depleted during delay");
    assertTrue(approxEqual(sys.getRemainingOre("adt1"), 0.0f), "0 ore during delay");
    // Advance past delay (5 more seconds = 10 total) + 1s respawn
    sys.update(6.0f);
    // After delay, 1s of respawn at 50 ore/s * security_bonus(1.0) = 50 ore
    float remaining = sys.getRemainingOre("adt1");
    assertTrue(remaining > 0.0f, "Ore respawning after delay");
}

static void testAsteroidDepletionTrackerActiveMiners() {
    std::cout << "\n=== AsteroidDepletionTracker: ActiveMiners ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    world.createEntity("adt1");
    sys.initialize("adt1");
    assertTrue(sys.setActiveMiners("adt1", 3), "Set 3 miners");
    assertTrue(sys.getActiveMiners("adt1") == 3, "3 active miners");
    sys.setActiveMiners("adt1", -1);  // clamp to 0
    assertTrue(sys.getActiveMiners("adt1") == 0, "Clamped to 0");
}

static void testAsteroidDepletionTrackerMissing() {
    std::cout << "\n=== AsteroidDepletionTracker: Missing ===" << std::endl;
    ecs::World world;
    systems::AsteroidDepletionTrackerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.extractOre("nonexistent", 100.0f), "Extract fails on missing");
    assertTrue(!sys.setActiveMiners("nonexistent", 1), "Set miners fails on missing");
    assertTrue(!sys.setSecurityBonus("nonexistent", 1.0f), "Security bonus fails on missing");
    assertTrue(!sys.setRespawnRate("nonexistent", 1.0f), "Respawn rate fails on missing");
    assertTrue(!sys.setRespawnDelay("nonexistent", 1.0f), "Respawn delay fails on missing");
    assertTrue(approxEqual(sys.getRemainingOre("nonexistent"), 0.0f), "0 ore on missing");
    assertTrue(approxEqual(sys.getTotalVolume("nonexistent"), 0.0f), "0 volume on missing");
    assertTrue(approxEqual(sys.getDepletionPercent("nonexistent"), 0.0f), "0% depletion on missing");
    assertTrue(!sys.isDepleted("nonexistent"), "Not depleted on missing");
    assertTrue(sys.getTimesDepleted("nonexistent") == 0, "0 depletions on missing");
    assertTrue(sys.getActiveMiners("nonexistent") == 0, "0 miners on missing");
    assertTrue(approxEqual(sys.getSecurityBonus("nonexistent"), 0.0f), "0 security on missing");
}

void run_asteroid_depletion_tracker_system_tests() {
    testAsteroidDepletionTrackerCreate();
    testAsteroidDepletionTrackerCustomVolume();
    testAsteroidDepletionTrackerExtract();
    testAsteroidDepletionTrackerFullDepletion();
    testAsteroidDepletionTrackerSecurityBonus();
    testAsteroidDepletionTrackerRespawn();
    testAsteroidDepletionTrackerActiveMiners();
    testAsteroidDepletionTrackerMissing();
}
