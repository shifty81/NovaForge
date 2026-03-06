// Tests for: Entity Lifecycle System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/entity_lifecycle_system.h"

using namespace atlas;

// ==================== Entity Lifecycle System Tests ====================

static void testEntityLifecycleCreate() {
    std::cout << "\n=== EntityLifecycle: Create ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1", "Frigate", "SpawnPoint-Alpha"), "Init succeeds");
    assertTrue(sys.getEventCount("e1") == 0, "No events initially");
    assertTrue(approxEqual(sys.getLifetime("e1"), 0.0f), "Lifetime is 0");
    assertTrue(sys.getDeathCause("e1") == 0, "No death cause");
    assertTrue(sys.getTotalSpawned("e1") == 0, "No spawns recorded");
    assertTrue(sys.getTotalDestroyed("e1") == 0, "No destroys recorded");
    assertTrue(sys.getTotalStateChanges("e1") == 0, "No state changes");
    assertTrue(sys.isAlive("e1"), "Alive initially");
    assertTrue(sys.getEntityType("e1") == "Frigate", "Entity type is Frigate");
    assertTrue(sys.getSpawnSource("e1") == "SpawnPoint-Alpha", "Spawn source correct");
}

static void testEntityLifecycleRecordSpawn() {
    std::cout << "\n=== EntityLifecycle: RecordSpawn ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "NPC", "Spawner-1");
    assertTrue(sys.recordSpawn("e1", "Frigate"), "Record spawn succeeds");
    assertTrue(sys.recordSpawn("e1", "Cruiser"), "Record another spawn");
    assertTrue(sys.getTotalSpawned("e1") == 2, "2 spawns recorded");
    assertTrue(sys.getEventCount("e1") == 2, "2 events recorded");
}

static void testEntityLifecycleRecordDestroy() {
    std::cout << "\n=== EntityLifecycle: RecordDestroy ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "Frigate", "Spawner-1");
    // DeathCause::Combat = 1
    assertTrue(sys.recordDestroy("e1", 1, "Killed by pirate"), "Record destroy succeeds");
    assertTrue(sys.getTotalDestroyed("e1") == 1, "1 destroy recorded");
    assertTrue(sys.getDeathCause("e1") == 1, "Death cause is Combat");
    assertTrue(!sys.isAlive("e1"), "No longer alive");
}

static void testEntityLifecycleRecordStateChange() {
    std::cout << "\n=== EntityLifecycle: RecordStateChange ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "NPC", "Spawner-1");
    assertTrue(sys.recordStateChange("e1", "Idle→Mining"), "Record state change");
    assertTrue(sys.recordStateChange("e1", "Mining→Hauling"), "Record another change");
    assertTrue(sys.getTotalStateChanges("e1") == 2, "2 state changes");
    assertTrue(sys.getEventCount("e1") == 2, "2 events");
}

static void testEntityLifecycleLifetimeTracking() {
    std::cout << "\n=== EntityLifecycle: LifetimeTracking ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "Frigate", "Spawner-1");
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getLifetime("e1"), 10.0f), "Lifetime is 10s");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getLifetime("e1"), 15.0f), "Lifetime is 15s");

    // After death, lifetime stops accumulating
    sys.markDead("e1", 1);
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getLifetime("e1"), 15.0f), "Lifetime frozen after death");
}

static void testEntityLifecycleMarkDead() {
    std::cout << "\n=== EntityLifecycle: MarkDead ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "Cruiser", "Spawner-1");
    assertTrue(sys.isAlive("e1"), "Alive initially");
    // DeathCause::Environmental = 4
    assertTrue(sys.markDead("e1", 4), "MarkDead succeeds");
    assertTrue(!sys.isAlive("e1"), "No longer alive");
    assertTrue(sys.getDeathCause("e1") == 4, "Death cause is Environmental");
}

static void testEntityLifecycleInvalidDeathCause() {
    std::cout << "\n=== EntityLifecycle: InvalidDeathCause ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "Ship", "Spawner-1");
    assertTrue(!sys.markDead("e1", -1), "Negative death cause rejected");
    assertTrue(!sys.markDead("e1", 6), "Out-of-range death cause rejected");
    assertTrue(!sys.recordDestroy("e1", -1, "invalid"), "Negative cause on destroy rejected");
    assertTrue(!sys.recordDestroy("e1", 6, "invalid"), "Out-of-range cause on destroy rejected");
}

static void testEntityLifecycleMaxEvents() {
    std::cout << "\n=== EntityLifecycle: MaxEvents ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1", "NPC", "Spawner-1");

    auto* entity = world.getEntity("e1");
    auto* lc = entity->getComponent<components::EntityLifecycle>();
    lc->max_events = 3;

    sys.recordSpawn("e1", "A");
    sys.recordSpawn("e1", "B");
    sys.recordSpawn("e1", "C");
    sys.recordSpawn("e1", "D");
    assertTrue(sys.getEventCount("e1") == 3, "Max events enforced (oldest evicted)");
    assertTrue(sys.getTotalSpawned("e1") == 4, "4 total spawns counted");
}

static void testEntityLifecycleMultipleEntities() {
    std::cout << "\n=== EntityLifecycle: MultipleEntities ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    world.createEntity("e1");
    world.createEntity("e2");
    sys.initialize("e1", "Frigate", "Spawner-1");
    sys.initialize("e2", "Cruiser", "Spawner-2");

    sys.recordSpawn("e1", "Frigate");
    sys.recordSpawn("e2", "Cruiser");
    sys.markDead("e1", 1);

    assertTrue(sys.getTotalSpawned("e1") == 1, "e1 has 1 spawn");
    assertTrue(sys.getTotalSpawned("e2") == 1, "e2 has 1 spawn");
    assertTrue(!sys.isAlive("e1"), "e1 is dead");
    assertTrue(sys.isAlive("e2"), "e2 is alive");
    assertTrue(sys.getEntityType("e2") == "Cruiser", "e2 is Cruiser");
}

static void testEntityLifecycleMissing() {
    std::cout << "\n=== EntityLifecycle: Missing ===" << std::endl;
    ecs::World world;
    systems::EntityLifecycleSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "Ship", "A"), "Init fails on missing");
    assertTrue(!sys.recordSpawn("nonexistent", "Ship"), "RecordSpawn fails on missing");
    assertTrue(!sys.recordDestroy("nonexistent", 1, "cause"), "RecordDestroy fails on missing");
    assertTrue(!sys.recordStateChange("nonexistent", "change"), "RecordStateChange fails on missing");
    assertTrue(!sys.markDead("nonexistent", 1), "MarkDead fails on missing");
    assertTrue(sys.getEventCount("nonexistent") == 0, "0 events on missing");
    assertTrue(approxEqual(sys.getLifetime("nonexistent"), 0.0f), "0 lifetime on missing");
    assertTrue(sys.getDeathCause("nonexistent") == 0, "0 death cause on missing");
    assertTrue(sys.getTotalSpawned("nonexistent") == 0, "0 spawns on missing");
    assertTrue(sys.getTotalDestroyed("nonexistent") == 0, "0 destroys on missing");
    assertTrue(sys.getTotalStateChanges("nonexistent") == 0, "0 changes on missing");
    assertTrue(!sys.isAlive("nonexistent"), "Not alive on missing");
    assertTrue(sys.getEntityType("nonexistent") == "", "Empty type on missing");
    assertTrue(sys.getSpawnSource("nonexistent") == "", "Empty source on missing");
}


void run_entity_lifecycle_system_tests() {
    testEntityLifecycleCreate();
    testEntityLifecycleRecordSpawn();
    testEntityLifecycleRecordDestroy();
    testEntityLifecycleRecordStateChange();
    testEntityLifecycleLifetimeTracking();
    testEntityLifecycleMarkDead();
    testEntityLifecycleInvalidDeathCause();
    testEntityLifecycleMaxEvents();
    testEntityLifecycleMultipleEntities();
    testEntityLifecycleMissing();
}
