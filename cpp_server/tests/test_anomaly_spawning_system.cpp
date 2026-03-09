// Tests for: Anomaly Spawning System
#include "test_log.h"
#include "components/exploration_components.h"
#include "systems/anomaly_spawning_system.h"

using namespace atlas;

// ==================== Anomaly Spawning System Tests ====================

static void testAnomalySpawnCreate() {
    std::cout << "\n=== AnomalySpawning: Create ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    assertTrue(sys.initialize("sys1", "alpha", 0.5f), "Init succeeds");
    assertTrue(sys.getAnomalyCount("sys1") == 0, "No anomalies initially");
    assertTrue(sys.getTotalSpawned("sys1") == 0, "0 spawned");
    assertTrue(sys.getTotalCompleted("sys1") == 0, "0 completed");
    assertTrue(sys.getTotalDespawned("sys1") == 0, "0 despawned");
    assertTrue(sys.isActive("sys1"), "Active by default");
}

static void testAnomalySpawnSpawn() {
    std::cout << "\n=== AnomalySpawning: Spawn ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "alpha", 0.5f);
    assertTrue(sys.spawnAnomaly("sys1", "anom1",
        components::AnomalySpawningState::AnomalyType::Combat, 3), "Spawn combat anomaly");
    assertTrue(sys.getAnomalyCount("sys1") == 1, "1 anomaly");
    assertTrue(sys.getTotalSpawned("sys1") == 1, "1 total spawned");
    assertTrue(sys.spawnAnomaly("sys1", "anom2",
        components::AnomalySpawningState::AnomalyType::Gas, 1), "Spawn gas anomaly");
    assertTrue(sys.getAnomalyCount("sys1") == 2, "2 anomalies");
    assertTrue(!sys.spawnAnomaly("sys1", "anom1",
        components::AnomalySpawningState::AnomalyType::Relic, 1), "Duplicate ID rejected");
}

static void testAnomalySpawnComplete() {
    std::cout << "\n=== AnomalySpawning: Complete ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "alpha", 0.5f);
    sys.spawnAnomaly("sys1", "anom1");
    assertTrue(sys.completeAnomaly("sys1", "anom1"), "Complete succeeds");
    assertTrue(sys.getTotalCompleted("sys1") == 1, "1 completed");
    assertTrue(!sys.completeAnomaly("sys1", "anom1"), "Double complete fails");
    assertTrue(!sys.completeAnomaly("sys1", "nonexistent"), "Complete nonexistent fails");
}

static void testAnomalySpawnRemove() {
    std::cout << "\n=== AnomalySpawning: Remove ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "alpha", 0.5f);
    sys.spawnAnomaly("sys1", "anom1");
    assertTrue(sys.removeAnomaly("sys1", "anom1"), "Remove succeeds");
    assertTrue(sys.getAnomalyCount("sys1") == 0, "0 anomalies after remove");
    assertTrue(!sys.removeAnomaly("sys1", "anom1"), "Double remove fails");
}

static void testAnomalySpawnMaxLimit() {
    std::cout << "\n=== AnomalySpawning: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "alpha", 0.5f);

    // At 0.5 security, max = round(3 * (1 + 2*(1-0.5))) = round(3*2) = 6
    int max_anoms = sys.getMaxAnomalies("sys1");
    // Need to trigger update to recalculate
    sys.update(0.1f);
    max_anoms = sys.getMaxAnomalies("sys1");
    assertTrue(max_anoms == 6, "Max anomalies = 6 at 0.5 sec");

    // Fill to max
    for (int i = 0; i < max_anoms; i++) {
        assertTrue(sys.spawnAnomaly("sys1", "a" + std::to_string(i)), "Spawn " + std::to_string(i));
    }
    assertTrue(!sys.spawnAnomaly("sys1", "overflow"), "Overflow rejected");
}

static void testAnomalySpawnSecurityScaling() {
    std::cout << "\n=== AnomalySpawning: SecurityScaling ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);

    // Null-sec (0.0) → max = round(3 * 3) = 9
    world.createEntity("null");
    sys.initialize("null", "null_sys", 0.0f);
    sys.update(0.1f);
    assertTrue(sys.getMaxAnomalies("null") == 9, "Null-sec: 9 max anomalies");

    // High-sec (1.0) → max = round(3 * 1) = 3
    world.createEntity("high");
    sys.initialize("high", "high_sys", 1.0f);
    sys.update(0.1f);
    assertTrue(sys.getMaxAnomalies("high") == 3, "High-sec: 3 max anomalies");
}

static void testAnomalySpawnDespawn() {
    std::cout << "\n=== AnomalySpawning: Despawn ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "alpha", 0.5f);
    sys.spawnAnomaly("sys1", "anom1");
    sys.completeAnomaly("sys1", "anom1");

    // Trigger despawn check (default interval = 60s)
    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::AnomalySpawningState>();
    state->despawn_check_interval = 1.0f;

    sys.update(1.1f);
    assertTrue(sys.getAnomalyCount("sys1") == 0, "Completed anomaly despawned");
    assertTrue(sys.getTotalDespawned("sys1") == 1, "1 despawned");
}

static void testAnomalySpawnInactive() {
    std::cout << "\n=== AnomalySpawning: Inactive ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");
    assertTrue(sys.setActive("sys1", false), "Deactivate succeeds");
    assertTrue(!sys.isActive("sys1"), "Not active");
    assertTrue(!sys.spawnAnomaly("sys1", "anom1"), "Can't spawn when inactive");
}

static void testAnomalySpawnDifficultyClamped() {
    std::cout << "\n=== AnomalySpawning: DifficultyClamped ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1", "alpha", 0.5f);
    sys.spawnAnomaly("sys1", "anom1",
        components::AnomalySpawningState::AnomalyType::Combat, 10);

    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::AnomalySpawningState>();
    assertTrue(state->anomalies[0].difficulty == 5, "Difficulty clamped to 5");

    sys.spawnAnomaly("sys1", "anom2",
        components::AnomalySpawningState::AnomalyType::Data, -1);
    assertTrue(state->anomalies[1].difficulty == 1, "Difficulty clamped to 1");
}

static void testAnomalySpawnMissing() {
    std::cout << "\n=== AnomalySpawning: Missing ===" << std::endl;
    ecs::World world;
    systems::AnomalySpawningSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.spawnAnomaly("nonexistent", "anom1"), "Spawn fails on missing");
    assertTrue(!sys.completeAnomaly("nonexistent", "anom1"), "Complete fails on missing");
    assertTrue(!sys.removeAnomaly("nonexistent", "anom1"), "Remove fails on missing");
    assertTrue(sys.getAnomalyCount("nonexistent") == 0, "0 anomalies on missing");
    assertTrue(sys.getTotalSpawned("nonexistent") == 0, "0 spawned on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(sys.getTotalDespawned("nonexistent") == 0, "0 despawned on missing");
    assertTrue(sys.getMaxAnomalies("nonexistent") == 0, "0 max on missing");
    assertTrue(!sys.setActive("nonexistent", true), "SetActive fails on missing");
    assertTrue(!sys.isActive("nonexistent"), "Not active on missing");
}

void run_anomaly_spawning_system_tests() {
    testAnomalySpawnCreate();
    testAnomalySpawnSpawn();
    testAnomalySpawnComplete();
    testAnomalySpawnRemove();
    testAnomalySpawnMaxLimit();
    testAnomalySpawnSecurityScaling();
    testAnomalySpawnDespawn();
    testAnomalySpawnInactive();
    testAnomalySpawnDifficultyClamped();
    testAnomalySpawnMissing();
}
