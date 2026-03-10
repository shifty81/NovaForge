// Tests for: PlayerSpawnSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/player_spawn_system.h"

using namespace atlas;

// ==================== PlayerSpawnSystem Tests ====================

static void testPlayerSpawnInit() {
    std::cout << "\n=== PlayerSpawn: Init ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "station_alpha"), "Init succeeds");
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Dead,
               "Initial state is Dead");
    assertTrue(!sys.isSpawned("p1"), "Not spawned initially");
    assertTrue(sys.getSpawnCount("p1") == 0, "Spawn count starts at 0");
    assertTrue(sys.getDeathCount("p1") == 0, "Death count starts at 0");
    assertTrue(approxEqual(sys.getRespawnTimer("p1"), 0.0f), "Respawn timer starts at 0");
    assertTrue(sys.getSpawnLocation("p1") == "station_alpha", "Spawn location set");
    assertTrue(sys.getDeathLocation("p1").empty(), "Death location starts empty");
}

static void testPlayerSpawnInitFails() {
    std::cout << "\n=== PlayerSpawn: InitFails ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "station"), "Init fails on missing entity");
    world.createEntity("p1");
    assertTrue(!sys.initialize("p1", ""), "Init fails with empty location");
}

static void testPlayerSpawnSpawnPlayer() {
    std::cout << "\n=== PlayerSpawn: SpawnPlayer ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");

    assertTrue(sys.spawnPlayer("p1"), "Spawn succeeds from Dead");
    assertTrue(sys.isSpawned("p1"), "Player is spawned");
    assertTrue(sys.getSpawnCount("p1") == 1, "Spawn count incremented to 1");
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Spawned,
               "State is Spawned");

    // Cannot spawn twice
    assertTrue(!sys.spawnPlayer("p1"), "Double spawn rejected");
    assertTrue(sys.getSpawnCount("p1") == 1, "Spawn count unchanged after rejection");
}

static void testPlayerSpawnKillPlayer() {
    std::cout << "\n=== PlayerSpawn: KillPlayer ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");
    sys.spawnPlayer("p1");

    assertTrue(sys.killPlayer("p1", "Jita IV-4"), "Kill player succeeds from Spawned");
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Dead,
               "State is Dead after kill");
    assertTrue(sys.getDeathCount("p1") == 1, "Death count incremented");
    assertTrue(sys.getDeathLocation("p1") == "Jita IV-4", "Death location recorded");

    // Cannot kill a dead player
    assertTrue(!sys.killPlayer("p1", "somewhere"), "Kill dead player rejected");
    assertTrue(sys.getDeathCount("p1") == 1, "Death count unchanged after rejection");
}

static void testPlayerSpawnBeginRespawn() {
    std::cout << "\n=== PlayerSpawn: BeginRespawn ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");
    sys.spawnPlayer("p1");
    sys.killPlayer("p1", "Jita IV-4");

    assertTrue(sys.beginRespawn("p1"), "Begin respawn succeeds from Dead");
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Respawning,
               "State is Respawning");
    assertTrue(approxEqual(sys.getRespawnTimer("p1"), 15.0f), "Respawn timer set to cooldown");

    // Cannot begin respawn again while already respawning
    assertTrue(!sys.beginRespawn("p1"), "Double begin respawn rejected");
}

static void testPlayerSpawnAutoRespawn() {
    std::cout << "\n=== PlayerSpawn: AutoRespawn ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");
    sys.spawnPlayer("p1");
    sys.killPlayer("p1", "Jita IV-4");

    // Set a short cooldown for testing
    sys.setRespawnCooldown("p1", 5.0f);
    sys.beginRespawn("p1");

    sys.update(3.0f);
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Respawning,
               "Still respawning after 3s");
    assertTrue(approxEqual(sys.getRespawnTimer("p1"), 2.0f), "Timer ticked down to 2s");

    sys.update(3.0f);
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Spawned,
               "Auto-spawned after cooldown");
    assertTrue(sys.getSpawnCount("p1") == 2, "Spawn count incremented by auto-respawn");
    assertTrue(approxEqual(sys.getRespawnTimer("p1"), 0.0f), "Timer reset to 0");
}

static void testPlayerSpawnMaxRespawnAttempts() {
    std::cout << "\n=== PlayerSpawn: MaxRespawnAttempts ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");

    auto* comp = world.getEntity("p1")->getComponent<components::PlayerSpawn>();
    comp->max_respawn_attempts = 2;
    sys.setRespawnCooldown("p1", 0.1f);

    // First death cycle
    sys.spawnPlayer("p1");
    sys.killPlayer("p1", "Amarr");
    assertTrue(sys.beginRespawn("p1"), "First respawn attempt");
    sys.update(1.0f); // auto-respawn

    // Second death cycle
    sys.killPlayer("p1", "Caldari");
    assertTrue(sys.beginRespawn("p1"), "Second respawn attempt");
    sys.update(1.0f);

    // Third death cycle — max_respawn_attempts reached
    sys.killPlayer("p1", "Gallente");
    assertTrue(!sys.beginRespawn("p1"), "Third respawn rejected: max attempts reached");
    assertTrue(sys.getState("p1") == components::PlayerSpawn::SpawnState::Dead,
               "Still Dead after rejected respawn");
}

static void testPlayerSpawnSetSpawnLocation() {
    std::cout << "\n=== PlayerSpawn: SetSpawnLocation ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");

    assertTrue(sys.setSpawnLocation("p1", "station_beta"), "Update spawn location");
    assertTrue(sys.getSpawnLocation("p1") == "station_beta", "New spawn location stored");
    assertTrue(!sys.setSpawnLocation("p1", ""), "Empty location rejected");
    assertTrue(sys.getSpawnLocation("p1") == "station_beta", "Location unchanged");
    assertTrue(!sys.setSpawnLocation("nonexistent", "station"), "Fails on missing entity");
}

static void testPlayerSpawnSetRespawnCooldown() {
    std::cout << "\n=== PlayerSpawn: SetRespawnCooldown ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "station_alpha");

    assertTrue(sys.setRespawnCooldown("p1", 30.0f), "Set cooldown to 30s");
    auto* comp = world.getEntity("p1")->getComponent<components::PlayerSpawn>();
    assertTrue(approxEqual(comp->respawn_cooldown, 30.0f), "Cooldown stored");

    assertTrue(!sys.setRespawnCooldown("p1", -1.0f), "Negative cooldown rejected");
    assertTrue(!sys.setRespawnCooldown("nonexistent", 10.0f), "Fails on missing entity");
}

static void testPlayerSpawnMissing() {
    std::cout << "\n=== PlayerSpawn: Missing ===" << std::endl;
    ecs::World world;
    systems::PlayerSpawnSystem sys(&world);

    assertTrue(!sys.spawnPlayer("nonexistent"), "Spawn fails on missing");
    assertTrue(!sys.killPlayer("nonexistent", "loc"), "Kill fails on missing");
    assertTrue(!sys.beginRespawn("nonexistent"), "BeginRespawn fails on missing");
    assertTrue(sys.getState("nonexistent") == components::PlayerSpawn::SpawnState::Dead,
               "Dead state on missing");
    assertTrue(!sys.isSpawned("nonexistent"), "Not spawned on missing");
    assertTrue(sys.getSpawnCount("nonexistent") == 0, "0 spawns on missing");
    assertTrue(sys.getDeathCount("nonexistent") == 0, "0 deaths on missing");
    assertTrue(approxEqual(sys.getRespawnTimer("nonexistent"), 0.0f), "0 timer on missing");
    assertTrue(sys.getSpawnLocation("nonexistent").empty(), "Empty location on missing");
    assertTrue(sys.getDeathLocation("nonexistent").empty(), "Empty death location on missing");
}

void run_player_spawn_system_tests() {
    testPlayerSpawnInit();
    testPlayerSpawnInitFails();
    testPlayerSpawnSpawnPlayer();
    testPlayerSpawnKillPlayer();
    testPlayerSpawnBeginRespawn();
    testPlayerSpawnAutoRespawn();
    testPlayerSpawnMaxRespawnAttempts();
    testPlayerSpawnSetSpawnLocation();
    testPlayerSpawnSetRespawnCooldown();
    testPlayerSpawnMissing();
}
