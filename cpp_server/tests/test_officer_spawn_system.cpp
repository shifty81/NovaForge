// Tests for: Officer Spawn System
#include "test_log.h"
#include "components/npc_components.h"
#include "systems/officer_spawn_system.h"

using namespace atlas;

// ==================== Officer Spawn System Tests ====================

static void testOfficerSpawnCreate() {
    std::cout << "\n=== OfficerSpawn: Create ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    assertTrue(sys.initialize("sys1", "jita"), "Init succeeds");
    assertTrue(sys.getAliveOfficerCount("sys1") == 0, "No officers initially");
    assertTrue(sys.getTotalSpawned("sys1") == 0, "0 spawned");
    assertTrue(sys.getTotalKilled("sys1") == 0, "0 killed");
    assertTrue(sys.isActive("sys1"), "Active by default");
    assertTrue(approxEqual(sys.getCurrentSpawnChance("sys1"), 0.001f), "Base spawn chance");
}

static void testOfficerSpawnSpawn() {
    std::cout << "\n=== OfficerSpawn: Spawn ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");
    assertTrue(sys.spawnOfficer("sys1", "serpentis",
        components::OfficerSpawnState::OfficerRank::Commander), "Spawn commander");
    assertTrue(sys.getAliveOfficerCount("sys1") == 1, "1 alive");
    assertTrue(sys.getTotalSpawned("sys1") == 1, "1 total spawned");
    // Default max_officers = 1, so second should fail
    assertTrue(!sys.spawnOfficer("sys1", "guristas",
        components::OfficerSpawnState::OfficerRank::Captain), "2nd spawn rejected (max 1)");
    assertTrue(sys.getAliveOfficerCount("sys1") == 1, "Still 1 alive");
}

static void testOfficerSpawnKill() {
    std::cout << "\n=== OfficerSpawn: Kill ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");
    sys.spawnOfficer("sys1", "serpentis",
        components::OfficerSpawnState::OfficerRank::Commander);

    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::OfficerSpawnState>();
    std::string off_id = state->officers[0].officer_id;

    assertTrue(sys.killOfficer("sys1", off_id), "Kill succeeds");
    assertTrue(sys.getAliveOfficerCount("sys1") == 0, "0 alive after kill");
    assertTrue(sys.getTotalKilled("sys1") == 1, "1 killed");
    assertTrue(!sys.killOfficer("sys1", off_id), "Double kill fails");
}

static void testOfficerSpawnRankLoot() {
    std::cout << "\n=== OfficerSpawn: RankLoot ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");

    auto* entity = world.getEntity("sys1");
    auto* state = entity->getComponent<components::OfficerSpawnState>();
    state->max_officers = 3;

    sys.spawnOfficer("sys1", "cmd", components::OfficerSpawnState::OfficerRank::Commander);
    sys.spawnOfficer("sys1", "cap", components::OfficerSpawnState::OfficerRank::Captain);
    sys.spawnOfficer("sys1", "adm", components::OfficerSpawnState::OfficerRank::Admiral);

    assertTrue(approxEqual(state->officers[0].bounty_multiplier, 5.0f), "Commander 5x bounty");
    assertTrue(approxEqual(state->officers[0].loot_quality, 1.0f), "Commander faction loot");
    assertTrue(approxEqual(state->officers[1].bounty_multiplier, 10.0f), "Captain 10x bounty");
    assertTrue(approxEqual(state->officers[1].loot_quality, 2.0f), "Captain officer loot");
    assertTrue(approxEqual(state->officers[2].bounty_multiplier, 20.0f), "Admiral 20x bounty");
    assertTrue(approxEqual(state->officers[2].loot_quality, 3.0f), "Admiral deadspace loot");
}

static void testOfficerSpawnChanceIncrease() {
    std::cout << "\n=== OfficerSpawn: ChanceIncrease ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");

    // After 10 minutes (600s), chance should have increased
    sys.update(600.0f);
    float chance = sys.getCurrentSpawnChance("sys1");
    assertTrue(chance > 0.001f, "Spawn chance increased after 10 min");
    assertTrue(approxEqual(sys.getTimeSinceLastSpawn("sys1"), 600.0f), "Time since last spawn = 600");
}

static void testOfficerSpawnChanceReset() {
    std::cout << "\n=== OfficerSpawn: ChanceReset ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");

    sys.update(600.0f);
    float before = sys.getCurrentSpawnChance("sys1");
    assertTrue(before > 0.001f, "Chance elevated before spawn");

    sys.spawnOfficer("sys1", "serpentis",
        components::OfficerSpawnState::OfficerRank::Commander);
    assertTrue(approxEqual(sys.getCurrentSpawnChance("sys1"), 0.001f), "Chance reset after spawn");
    assertTrue(approxEqual(sys.getTimeSinceLastSpawn("sys1"), 0.0f), "Time reset after spawn");
}

static void testOfficerSpawnInactive() {
    std::cout << "\n=== OfficerSpawn: Inactive ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("sys1");
    sys.initialize("sys1");
    assertTrue(sys.setActive("sys1", false), "Deactivate succeeds");
    assertTrue(!sys.isActive("sys1"), "Not active");
    assertTrue(!sys.spawnOfficer("sys1", "test",
        components::OfficerSpawnState::OfficerRank::Commander), "Can't spawn when inactive");
}

static void testOfficerSpawnMissing() {
    std::cout << "\n=== OfficerSpawn: Missing ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.spawnOfficer("nonexistent", "test"), "Spawn fails on missing");
    assertTrue(!sys.killOfficer("nonexistent", "test"), "Kill fails on missing");
    assertTrue(sys.getAliveOfficerCount("nonexistent") == 0, "0 alive on missing");
    assertTrue(sys.getTotalSpawned("nonexistent") == 0, "0 spawned on missing");
    assertTrue(sys.getTotalKilled("nonexistent") == 0, "0 killed on missing");
    assertTrue(approxEqual(sys.getCurrentSpawnChance("nonexistent"), 0.0f), "0 chance on missing");
    assertTrue(approxEqual(sys.getTimeSinceLastSpawn("nonexistent"), 0.0f), "0 time on missing");
    assertTrue(!sys.setActive("nonexistent", true), "SetActive fails on missing");
    assertTrue(!sys.isActive("nonexistent"), "Not active on missing");
}

void run_officer_spawn_system_tests() {
    testOfficerSpawnCreate();
    testOfficerSpawnSpawn();
    testOfficerSpawnKill();
    testOfficerSpawnRankLoot();
    testOfficerSpawnChanceIncrease();
    testOfficerSpawnChanceReset();
    testOfficerSpawnInactive();
    testOfficerSpawnMissing();
}
