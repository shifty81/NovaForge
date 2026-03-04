// Tests for: FPS Spawn System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fps_spawn_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== FPS Spawn System Tests ====================

static void testFPSSpawnCreate() {
    std::cout << "\n=== FPS Spawn Create ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);

    assertTrue(sys.createSpawnPoint("spawn1", "station1",
                                     components::FPSSpawnPoint::SpawnContext::Hangar,
                                     10.0f, 0.0f, 2.0f, 90.0f),
               "Spawn point created");
    assertTrue(!sys.createSpawnPoint("spawn1", "station1",
                                      components::FPSSpawnPoint::SpawnContext::Hangar,
                                      0, 0, 0),
               "Duplicate spawn fails");
}

static void testFPSSpawnTransform() {
    std::cout << "\n=== FPS Spawn Transform ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);
    sys.createSpawnPoint("spawn1", "station1",
                         components::FPSSpawnPoint::SpawnContext::Hangar,
                         10.0f, 5.0f, 2.0f, 90.0f);

    auto [x, y, z, yaw] = sys.getSpawnTransform("spawn1");
    assertTrue(approxEqual(x, 10.0f), "Spawn X");
    assertTrue(approxEqual(y, 5.0f), "Spawn Y");
    assertTrue(approxEqual(z, 2.0f), "Spawn Z");
    assertTrue(approxEqual(yaw, 90.0f), "Spawn Yaw");
}

static void testFPSSpawnContext() {
    std::cout << "\n=== FPS Spawn Context ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);
    sys.createSpawnPoint("spawn1", "station1",
                         components::FPSSpawnPoint::SpawnContext::TetherAirlock,
                         0, 0, 0);

    assertTrue(sys.getSpawnContext("spawn1") ==
               components::FPSSpawnPoint::SpawnContext::TetherAirlock,
               "Context is TetherAirlock");
}

static void testFPSSpawnActivation() {
    std::cout << "\n=== FPS Spawn Activation ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);
    sys.createSpawnPoint("spawn1", "station1",
                         components::FPSSpawnPoint::SpawnContext::Hangar,
                         0, 0, 0);

    assertTrue(sys.setSpawnActive("spawn1", false), "Deactivate succeeds");
    assertTrue(sys.setSpawnActive("spawn1", true), "Reactivate succeeds");
    assertTrue(!sys.setSpawnActive("nonexistent", false), "Nonexistent fails");
}

static void testFPSSpawnFindForDockedPlayer() {
    std::cout << "\n=== FPS Spawn Find For Docked Player ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);

    // Create station and player.
    world.createEntity("station1");
    auto* player = world.createEntity("player1");
    auto docked = std::make_unique<components::Docked>();
    docked->station_id = "station1";
    player->addComponent(std::move(docked));

    // Create a hangar owned by this player at the station.
    auto* hangarEnt = world.createEntity("hangar1");
    auto hangarComp = std::make_unique<components::StationHangar>();
    hangarComp->station_id = "station1";
    hangarComp->owner_id = "player1";
    hangarEnt->addComponent(std::move(hangarComp));

    // Create hangar and lobby spawn points.
    sys.createSpawnPoint("sp_hangar", "station1",
                         components::FPSSpawnPoint::SpawnContext::Hangar,
                         10.0f, 0.0f, 2.0f);
    sys.createSpawnPoint("sp_lobby", "station1",
                         components::FPSSpawnPoint::SpawnContext::StationLobby,
                         0.0f, 0.0f, 0.0f);

    std::string spawn = sys.findSpawnForPlayer("player1");
    assertTrue(spawn == "sp_hangar", "Docked player with hangar spawns in hangar");
}

static void testFPSSpawnFindForDockedPlayerNoHangar() {
    std::cout << "\n=== FPS Spawn Find For Docked Player (No Hangar) ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);

    world.createEntity("station1");
    auto* player = world.createEntity("player1");
    auto docked = std::make_unique<components::Docked>();
    docked->station_id = "station1";
    player->addComponent(std::move(docked));

    // Only a lobby spawn, no hangar.
    sys.createSpawnPoint("sp_lobby", "station1",
                         components::FPSSpawnPoint::SpawnContext::StationLobby,
                         0.0f, 0.0f, 0.0f);

    std::string spawn = sys.findSpawnForPlayer("player1");
    assertTrue(spawn == "sp_lobby", "Docked player without hangar spawns in lobby");
}

static void testFPSSpawnContextNames() {
    std::cout << "\n=== FPS Spawn Context Names ===" << std::endl;
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::Hangar) == "Hangar", "Hangar name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::StationLobby) == "StationLobby", "StationLobby name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::ShipBridge) == "ShipBridge", "ShipBridge name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::TetherAirlock) == "TetherAirlock", "TetherAirlock name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::EVAHatch) == "EVAHatch", "EVAHatch name");
}

static void testFPSSpawnComponentDefaults() {
    std::cout << "\n=== FPS Spawn Component Defaults ===" << std::endl;
    components::FPSSpawnPoint sp;
    assertTrue(sp.context == components::FPSSpawnPoint::SpawnContext::Hangar, "Default context Hangar");
    assertTrue(sp.is_active, "Default active");
    assertTrue(approxEqual(sp.pos_x, 0.0f), "Default pos_x 0");
    assertTrue(approxEqual(sp.yaw, 0.0f), "Default yaw 0");
}


void run_fps_spawn_system_tests() {
    testFPSSpawnCreate();
    testFPSSpawnTransform();
    testFPSSpawnContext();
    testFPSSpawnActivation();
    testFPSSpawnFindForDockedPlayer();
    testFPSSpawnFindForDockedPlayerNoHangar();
    testFPSSpawnContextNames();
    testFPSSpawnComponentDefaults();
}
