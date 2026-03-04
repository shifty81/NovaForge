// Tests for: StationSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/station_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== StationSystem Tests ====================

static void testStationCreate() {
    std::cout << "\n=== Station Create ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    bool ok = stationSys.createStation("station_1", "Test Hub", 100.0f, 0.0f, 200.0f, 3000.0f, 2.0f);
    assertTrue(ok, "Station created successfully");

    auto* entity = world.getEntity("station_1");
    assertTrue(entity != nullptr, "Station entity exists");

    auto* station = entity->getComponent<components::Station>();
    assertTrue(station != nullptr, "Station component attached");
    assertTrue(station->station_name == "Test Hub", "Station name is correct");
    assertTrue(approxEqual(station->docking_range, 3000.0f), "Docking range is correct");
    assertTrue(approxEqual(station->repair_cost_per_hp, 2.0f), "Repair cost per HP is correct");
}

static void testStationDuplicateCreate() {
    std::cout << "\n=== Station Duplicate Create ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub A", 0, 0, 0);
    bool dup = stationSys.createStation("station_1", "Hub B", 0, 0, 0);
    assertTrue(!dup, "Duplicate station creation rejected");
}

static void testStationDockInRange() {
    std::cout << "\n=== Station Dock In Range ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 100.0f;
    addComp<components::Velocity>(ship);
    addComp<components::Player>(ship);

    bool ok = stationSys.dockAtStation("player_1", "station_1");
    assertTrue(ok, "Docking succeeds when in range");
    assertTrue(stationSys.isDocked("player_1"), "Player is docked");
    assertTrue(stationSys.getDockedStation("player_1") == "station_1", "Docked at correct station");
}

static void testStationDockOutOfRange() {
    std::cout << "\n=== Station Dock Out Of Range ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 500.0f);

    auto* ship = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 9999.0f;
    addComp<components::Velocity>(ship);

    bool ok = stationSys.dockAtStation("player_1", "station_1");
    assertTrue(!ok, "Docking fails when out of range");
    assertTrue(!stationSys.isDocked("player_1"), "Player is not docked");
}

static void testStationUndock() {
    std::cout << "\n=== Station Undock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    stationSys.dockAtStation("player_1", "station_1");
    assertTrue(stationSys.isDocked("player_1"), "Docked before undock");

    bool ok = stationSys.undockFromStation("player_1");
    assertTrue(ok, "Undock succeeds");
    assertTrue(!stationSys.isDocked("player_1"), "No longer docked after undock");
}

static void testStationUndockNotDocked() {
    std::cout << "\n=== Station Undock Not Docked ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);

    bool ok = stationSys.undockFromStation("player_1");
    assertTrue(!ok, "Undock fails when not docked");
}

static void testStationRepair() {
    std::cout << "\n=== Station Repair ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f, 1.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = 50.0f;  hp->shield_max = 100.0f;
    hp->armor_hp  = 30.0f;  hp->armor_max  = 100.0f;
    hp->hull_hp   = 80.0f;  hp->hull_max   = 100.0f;

    auto* player = addComp<components::Player>(ship);
    player->credits = 10000.0;

    stationSys.dockAtStation("player_1", "station_1");

    double cost = stationSys.repairShip("player_1");
    // Damage = (100-50) + (100-30) + (100-80) = 50+70+20 = 140 HP, at 1 Credits/hp = 140
    assertTrue(approxEqual(static_cast<float>(cost), 140.0f), "Repair cost is 140 Credits");
    assertTrue(approxEqual(hp->shield_hp, 100.0f), "Shield fully repaired");
    assertTrue(approxEqual(hp->armor_hp, 100.0f), "Armor fully repaired");
    assertTrue(approxEqual(hp->hull_hp, 100.0f), "Hull fully repaired");
    assertTrue(approxEqual(static_cast<float>(player->credits), 9860.0f), "Credits deducted");
}

static void testStationRepairNoDamage() {
    std::cout << "\n=== Station Repair No Damage ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = hp->shield_max = 100.0f;
    hp->armor_hp  = hp->armor_max  = 100.0f;
    hp->hull_hp   = hp->hull_max   = 100.0f;

    addComp<components::Player>(ship);

    stationSys.dockAtStation("player_1", "station_1");

    double cost = stationSys.repairShip("player_1");
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No cost when no damage");
}

static void testStationRepairNotDocked() {
    std::cout << "\n=== Station Repair Not Docked ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = 50.0f; hp->shield_max = 100.0f;

    double cost = stationSys.repairShip("player_1");
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No repair when not docked");
}

static void testStationDockedCount() {
    std::cout << "\n=== Station Docked Count ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* s1 = world.createEntity("p1");
    addComp<components::Position>(s1);
    addComp<components::Velocity>(s1);

    auto* s2 = world.createEntity("p2");
    addComp<components::Position>(s2);
    addComp<components::Velocity>(s2);

    stationSys.dockAtStation("p1", "station_1");
    stationSys.dockAtStation("p2", "station_1");

    auto* station = world.getEntity("station_1")->getComponent<components::Station>();
    assertTrue(station->docked_count == 2, "Two ships docked");

    stationSys.undockFromStation("p1");
    assertTrue(station->docked_count == 1, "One ship after undock");
}

static void testStationDoubleDock() {
    std::cout << "\n=== Station Double Dock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("p1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    stationSys.dockAtStation("p1", "station_1");
    bool again = stationSys.dockAtStation("p1", "station_1");
    assertTrue(!again, "Cannot dock when already docked");
}

static void testStationMovementStopsOnDock() {
    std::cout << "\n=== Station Movement Stops On Dock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("p1");
    addComp<components::Position>(ship);
    auto* vel = addComp<components::Velocity>(ship);
    vel->vx = 100.0f;
    vel->vy = 50.0f;
    vel->vz = 200.0f;

    stationSys.dockAtStation("p1", "station_1");
    assertTrue(approxEqual(vel->vx, 0.0f), "Velocity X zeroed on dock");
    assertTrue(approxEqual(vel->vy, 0.0f), "Velocity Y zeroed on dock");
    assertTrue(approxEqual(vel->vz, 0.0f), "Velocity Z zeroed on dock");
}


void run_station_system_tests() {
    testStationCreate();
    testStationDuplicateCreate();
    testStationDockInRange();
    testStationDockOutOfRange();
    testStationUndock();
    testStationUndockNotDocked();
    testStationRepair();
    testStationRepairNoDamage();
    testStationRepairNotDocked();
    testStationDockedCount();
    testStationDoubleDock();
    testStationMovementStopsOnDock();
}
