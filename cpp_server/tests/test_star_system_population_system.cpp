// Tests for: StarSystemPopulationSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/star_system_population_system.h"

using namespace atlas;

// ==================== StarSystemPopulationSystem Tests ====================

static void testPopulationDefaultState() {
    std::cout << "\n=== StarSystemPopulation: DefaultState ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    auto* state = addComp<components::StarSystemPopulationState>(e);

    assertTrue(state->max_npcs == 50, "Default max NPCs 50");
    assertTrue(state->current_npcs == 0, "Zero NPCs initially");
    assertTrue(sys.getCurrentPopulation("system1") == 0, "getCurrentPopulation zero");
    assertTrue(!sys.isAtCapacity("system1"), "Not at capacity");
    assertTrue(sys.getTotalSpawned("system1") == 0, "Zero spawned");
    assertTrue(sys.getTotalDespawned("system1") == 0, "Zero despawned");
    assertTrue(approxEqual(sys.getActivityLevel("system1"), 1.0f), "Default activity 1.0");
}

static void testPopulationSpawnNPC() {
    std::cout << "\n=== StarSystemPopulation: SpawnNPC ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    auto* state = addComp<components::StarSystemPopulationState>(e);
    state->max_npcs = 5;

    assertTrue(sys.spawnNPC("system1", "miner"), "Spawn miner");
    assertTrue(sys.getRoleCount("system1", "miner") == 1, "1 miner");
    assertTrue(sys.getCurrentPopulation("system1") == 1, "Population 1");

    assertTrue(sys.spawnNPC("system1", "hauler"), "Spawn hauler");
    assertTrue(sys.spawnNPC("system1", "trader"), "Spawn trader");
    assertTrue(sys.spawnNPC("system1", "pirate"), "Spawn pirate");
    assertTrue(sys.spawnNPC("system1", "security"), "Spawn security");

    assertTrue(sys.getCurrentPopulation("system1") == 5, "Population 5");
    assertTrue(sys.isAtCapacity("system1"), "At capacity");
    assertTrue(sys.getTotalSpawned("system1") == 5, "5 total spawned");
}

static void testPopulationCapacityLimit() {
    std::cout << "\n=== StarSystemPopulation: CapacityLimit ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    auto* state = addComp<components::StarSystemPopulationState>(e);
    state->max_npcs = 2;

    sys.spawnNPC("system1", "miner");
    sys.spawnNPC("system1", "miner");
    assertTrue(!sys.spawnNPC("system1", "miner"), "Cannot spawn past capacity");
    assertTrue(sys.getCurrentPopulation("system1") == 2, "Still at 2");
}

static void testPopulationDespawnNPC() {
    std::cout << "\n=== StarSystemPopulation: DespawnNPC ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    addComp<components::StarSystemPopulationState>(e);

    sys.spawnNPC("system1", "miner");
    sys.spawnNPC("system1", "miner");
    sys.spawnNPC("system1", "hauler");

    assertTrue(sys.despawnNPC("system1", "miner"), "Despawn miner");
    assertTrue(sys.getRoleCount("system1", "miner") == 1, "1 miner remaining");
    assertTrue(sys.getCurrentPopulation("system1") == 2, "Population 2");
    assertTrue(sys.getTotalDespawned("system1") == 1, "1 total despawned");

    // Cannot despawn role with zero count
    assertTrue(!sys.despawnNPC("system1", "pirate"), "Cannot despawn pirate (none active)");
}

static void testPopulationInvalidRole() {
    std::cout << "\n=== StarSystemPopulation: InvalidRole ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    addComp<components::StarSystemPopulationState>(e);

    assertTrue(!sys.spawnNPC("system1", "invalid_role"), "Spawn invalid role fails");
    assertTrue(!sys.despawnNPC("system1", "invalid_role"), "Despawn invalid role fails");
    assertTrue(sys.getRoleCount("system1", "invalid_role") == 0, "Invalid role count 0");
}

static void testPopulationUpdate() {
    std::cout << "\n=== StarSystemPopulation: Update ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    auto* state = addComp<components::StarSystemPopulationState>(e);
    state->spawn_interval = 5.0f;
    state->max_npcs = 10;
    state->activity_level = 1.0f;

    // Before interval — no spawn
    sys.update(3.0f);
    assertTrue(sys.getCurrentPopulation("system1") == 0, "No spawn before interval");

    // At interval — spawn one
    sys.update(3.0f);
    assertTrue(sys.getCurrentPopulation("system1") == 1, "One NPC spawned at interval");
    assertTrue(sys.getTotalSpawned("system1") == 1, "1 total spawned");
}

static void testPopulationHighActivity() {
    std::cout << "\n=== StarSystemPopulation: HighActivity ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    auto* state = addComp<components::StarSystemPopulationState>(e);
    state->spawn_interval = 10.0f;
    state->activity_level = 2.0f;  // double rate
    state->max_npcs = 10;

    // Effective interval = 10/2 = 5s
    sys.update(5.0f);
    assertTrue(sys.getCurrentPopulation("system1") == 1, "Spawn at half interval with 2x activity");
}

static void testPopulationRoleDistribution() {
    std::cout << "\n=== StarSystemPopulation: RoleDistribution ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);
    auto* e = world.createEntity("system1");
    auto* state = addComp<components::StarSystemPopulationState>(e);
    state->spawn_interval = 1.0f;
    state->max_npcs = 20;

    // Spawn 20 NPCs via update ticks
    for (int i = 0; i < 20; i++) {
        sys.update(1.0f);
    }

    // 30% miners (6), 20% haulers (4), 20% traders (4), 15% pirates (3), 15% security (3)
    assertTrue(sys.getRoleCount("system1", "miner") == 6, "6 miners from ratio");
    assertTrue(sys.getRoleCount("system1", "hauler") == 4, "4 haulers from ratio");
    assertTrue(sys.getRoleCount("system1", "trader") == 4, "4 traders from ratio");
    assertTrue(sys.getRoleCount("system1", "pirate") == 3, "3 pirates from ratio");
    assertTrue(sys.getRoleCount("system1", "security") == 3, "3 security from ratio");
    assertTrue(sys.getCurrentPopulation("system1") == 20, "Total population 20");
}

static void testPopulationMissing() {
    std::cout << "\n=== StarSystemPopulation: Missing ===" << std::endl;
    ecs::World world;
    systems::StarSystemPopulationSystem sys(&world);

    assertTrue(!sys.spawnNPC("x", "miner"), "Spawn on missing");
    assertTrue(!sys.despawnNPC("x", "miner"), "Despawn on missing");
    assertTrue(sys.getCurrentPopulation("x") == 0, "Population on missing");
    assertTrue(sys.getRoleCount("x", "miner") == 0, "Role count on missing");
    assertTrue(!sys.isAtCapacity("x"), "Capacity on missing");
    assertTrue(sys.getTotalSpawned("x") == 0, "Spawned on missing");
    assertTrue(sys.getTotalDespawned("x") == 0, "Despawned on missing");
    assertTrue(approxEqual(sys.getActivityLevel("x"), 0.0f), "Activity on missing");
}

void run_star_system_population_system_tests() {
    testPopulationDefaultState();
    testPopulationSpawnNPC();
    testPopulationCapacityLimit();
    testPopulationDespawnNPC();
    testPopulationInvalidRole();
    testPopulationUpdate();
    testPopulationHighActivity();
    testPopulationRoleDistribution();
    testPopulationMissing();
}
