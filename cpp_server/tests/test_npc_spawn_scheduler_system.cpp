// Tests for: NPC Spawn Scheduler System
#include "test_log.h"
#include "components/core_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/npc_spawn_scheduler_system.h"

using namespace atlas;

// ==================== NPC Spawn Scheduler System Tests ====================

static void testNpcSpawnSchedulerCreate() {
    std::cout << "\n=== NpcSpawnScheduler: Create ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    assertTrue(sys.initialize("belt1", 10, 30.0f), "Init succeeds");
    assertTrue(sys.getLiveCount("belt1") == 0, "0 live");
    assertTrue(sys.getPopulationCap("belt1") == 10, "Cap is 10");
    assertTrue(sys.getWaveEntryCount("belt1") == 0, "0 waves");
    assertTrue(sys.getCurrentWaveIndex("belt1") == 0, "Wave index 0");
    assertTrue(sys.getTotalSpawned("belt1") == 0, "0 spawned");
    assertTrue(sys.getTotalKilled("belt1") == 0, "0 killed");
    assertTrue(!sys.isPaused("belt1"), "Not paused");

    // Invalid params rejected
    assertTrue(!sys.initialize("belt1", 0, 30.0f), "Cap 0 rejected");
    assertTrue(!sys.initialize("belt1", 10, 0.0f), "Interval 0 rejected");
}

static void testNpcSpawnSchedulerAddWave() {
    std::cout << "\n=== NpcSpawnScheduler: AddWave ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 10, 30.0f);

    assertTrue(sys.addWaveEntry("belt1", "pirate_frigate", 3), "Add pirate_frigate");
    assertTrue(sys.addWaveEntry("belt1", "pirate_cruiser", 2), "Add pirate_cruiser");
    assertTrue(sys.getWaveEntryCount("belt1") == 2, "2 wave entries");

    // Duplicate rejected
    assertTrue(!sys.addWaveEntry("belt1", "pirate_frigate", 5), "Dup rejected");

    // Invalid count
    assertTrue(!sys.addWaveEntry("belt1", "bad_npc", 0), "Count 0 rejected");
    assertTrue(!sys.addWaveEntry("belt1", "bad_npc", -1), "Negative count rejected");
}

static void testNpcSpawnSchedulerRemoveWave() {
    std::cout << "\n=== NpcSpawnScheduler: RemoveWave ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 10, 30.0f);
    sys.addWaveEntry("belt1", "pirate_frigate", 3);
    sys.addWaveEntry("belt1", "pirate_cruiser", 2);

    assertTrue(sys.removeWaveEntry("belt1", "pirate_frigate"), "Remove frigate");
    assertTrue(sys.getWaveEntryCount("belt1") == 1, "1 entry left");
    assertTrue(!sys.removeWaveEntry("belt1", "pirate_frigate"), "Can't remove twice");
    assertTrue(!sys.removeWaveEntry("belt1", "nonexistent"), "Can't remove missing");
}

static void testNpcSpawnSchedulerSpawning() {
    std::cout << "\n=== NpcSpawnScheduler: Spawning ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 10, 5.0f);  // 5s respawn
    sys.addWaveEntry("belt1", "pirate_frigate", 3);

    // Before respawn interval elapses
    sys.update(2.0f);
    assertTrue(sys.getLiveCount("belt1") == 0, "Not yet spawned at 2s");

    // After respawn interval
    sys.update(3.0f);
    assertTrue(sys.getLiveCount("belt1") == 3, "3 spawned at 5s");
    assertTrue(sys.getTotalSpawned("belt1") == 3, "3 total spawned");

    // Another wave after another interval
    sys.update(5.0f);
    assertTrue(sys.getLiveCount("belt1") == 6, "6 spawned at 10s");
}

static void testNpcSpawnSchedulerPopCap() {
    std::cout << "\n=== NpcSpawnScheduler: PopCap ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 5, 1.0f);  // cap 5, 1s respawn
    sys.addWaveEntry("belt1", "pirate_frigate", 4);

    // First wave: spawns 4
    sys.update(1.0f);
    assertTrue(sys.getLiveCount("belt1") == 4, "4 spawned (below cap)");

    // Second wave: only 1 more (cap is 5)
    sys.update(1.0f);
    assertTrue(sys.getLiveCount("belt1") == 5, "5 spawned (at cap)");

    // No more spawning at cap
    sys.update(1.0f);
    assertTrue(sys.getLiveCount("belt1") == 5, "Still 5 (at cap)");
}

static void testNpcSpawnSchedulerKill() {
    std::cout << "\n=== NpcSpawnScheduler: Kill ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 10, 1.0f);
    sys.addWaveEntry("belt1", "pirate_frigate", 3);
    sys.update(1.0f);  // spawn 3

    assertTrue(sys.recordKill("belt1"), "Kill recorded");
    assertTrue(sys.getLiveCount("belt1") == 2, "2 live after kill");
    assertTrue(sys.getTotalKilled("belt1") == 1, "1 total killed");

    // Kill remaining
    sys.recordKill("belt1");
    sys.recordKill("belt1");
    assertTrue(sys.getLiveCount("belt1") == 0, "0 live");

    // Can't kill below 0
    assertTrue(!sys.recordKill("belt1"), "Can't kill below 0");
}

static void testNpcSpawnSchedulerWaveCycling() {
    std::cout << "\n=== NpcSpawnScheduler: WaveCycling ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 20, 1.0f);
    sys.addWaveEntry("belt1", "pirate_frigate", 2);
    sys.addWaveEntry("belt1", "pirate_cruiser", 3);

    // First wave: frigate (index 0)
    sys.update(1.0f);
    assertTrue(sys.getCurrentWaveIndex("belt1") == 1, "Wave index advances to 1");
    assertTrue(sys.getLiveCount("belt1") == 2, "2 spawned (frigates)");

    // Second wave: cruiser (index 1)
    sys.update(1.0f);
    assertTrue(sys.getCurrentWaveIndex("belt1") == 0, "Wave index cycles back to 0");
    assertTrue(sys.getLiveCount("belt1") == 5, "5 spawned (2+3)");
}

static void testNpcSpawnSchedulerPauseResume() {
    std::cout << "\n=== NpcSpawnScheduler: PauseResume ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 10, 1.0f);
    sys.addWaveEntry("belt1", "pirate_frigate", 3);

    assertTrue(sys.pause("belt1"), "Pause succeeds");
    assertTrue(sys.isPaused("belt1"), "Is paused");
    assertTrue(!sys.pause("belt1"), "Can't pause twice");

    // No spawning while paused
    sys.update(5.0f);
    assertTrue(sys.getLiveCount("belt1") == 0, "No spawns while paused");

    assertTrue(sys.resume("belt1"), "Resume succeeds");
    assertTrue(!sys.isPaused("belt1"), "Not paused");
    assertTrue(!sys.resume("belt1"), "Can't resume twice");

    // Spawning resumes
    sys.update(1.0f);
    assertTrue(sys.getLiveCount("belt1") == 3, "Spawned after resume");
}

static void testNpcSpawnSchedulerMaxWaves() {
    std::cout << "\n=== NpcSpawnScheduler: MaxWaves ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    world.createEntity("belt1");
    sys.initialize("belt1", 100, 1.0f);

    // Fill to max (10)
    for (int i = 0; i < 10; i++) {
        assertTrue(sys.addWaveEntry("belt1", "npc_" + std::to_string(i), 1),
                   "Add npc_" + std::to_string(i));
    }
    assertTrue(sys.getWaveEntryCount("belt1") == 10, "10 wave entries at max");
    assertTrue(!sys.addWaveEntry("belt1", "npc_10", 1), "11th rejected");
}

static void testNpcSpawnSchedulerMissing() {
    std::cout << "\n=== NpcSpawnScheduler: Missing ===" << std::endl;
    ecs::World world;
    systems::NpcSpawnSchedulerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 10, 30.0f), "Init fails on missing");
    assertTrue(!sys.addWaveEntry("nonexistent", "pirate", 3), "AddWave fails on missing");
    assertTrue(!sys.removeWaveEntry("nonexistent", "pirate"), "RemoveWave fails on missing");
    assertTrue(!sys.recordKill("nonexistent"), "Kill fails on missing");
    assertTrue(!sys.pause("nonexistent"), "Pause fails on missing");
    assertTrue(!sys.resume("nonexistent"), "Resume fails on missing");
    assertTrue(sys.getLiveCount("nonexistent") == 0, "0 live on missing");
    assertTrue(sys.getPopulationCap("nonexistent") == 0, "0 cap on missing");
    assertTrue(sys.getWaveEntryCount("nonexistent") == 0, "0 waves on missing");
    assertTrue(sys.getTotalSpawned("nonexistent") == 0, "0 spawned on missing");
    assertTrue(sys.getTotalKilled("nonexistent") == 0, "0 killed on missing");
}

void run_npc_spawn_scheduler_system_tests() {
    testNpcSpawnSchedulerCreate();
    testNpcSpawnSchedulerAddWave();
    testNpcSpawnSchedulerRemoveWave();
    testNpcSpawnSchedulerSpawning();
    testNpcSpawnSchedulerPopCap();
    testNpcSpawnSchedulerKill();
    testNpcSpawnSchedulerWaveCycling();
    testNpcSpawnSchedulerPauseResume();
    testNpcSpawnSchedulerMaxWaves();
    testNpcSpawnSchedulerMissing();
}
