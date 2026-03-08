// Tests for: FleetProgressionSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_progression_system.h"

using namespace atlas;

// ==================== FleetProgressionSystem Tests ====================

static void testFleetProgressionInitialState() {
    std::cout << "\n=== FleetProgression: InitialState ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");

    // addExperience auto-creates component
    sys.addExperience("fleet1", 0.0f);

    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Early, "Starts in Early stage");
    assertTrue(sys.getMaxShips("fleet1") == 5, "Early max ships is 5");
    assertTrue(sys.getMaxWings("fleet1") == 1, "Early max wings is 1");
    assertTrue(sys.canAddShip("fleet1"), "Can add ship (1/5)");
}

static void testFleetProgressionMidStage() {
    std::cout << "\n=== FleetProgression: MidStage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");

    sys.addExperience("fleet1", 99.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Early, "99 XP still Early");
    assertTrue(sys.getMaxShips("fleet1") == 5, "Max ships still 5");

    sys.addExperience("fleet1", 1.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Mid, "100 XP reaches Mid");
    assertTrue(sys.getMaxShips("fleet1") == 15, "Mid max ships is 15");
    assertTrue(sys.getMaxWings("fleet1") == 3, "Mid max wings is 3");
}

static void testFleetProgressionEndStage() {
    std::cout << "\n=== FleetProgression: EndStage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");

    sys.addExperience("fleet1", 499.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Mid, "499 XP still Mid");

    sys.addExperience("fleet1", 1.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::End, "500 XP reaches End");
    assertTrue(sys.getMaxShips("fleet1") == 25, "End max ships is 25");
    assertTrue(sys.getMaxWings("fleet1") == 5, "End max wings is 5");
}

static void testFleetProgressionAddShip() {
    std::cout << "\n=== FleetProgression: AddShip ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 0.0f);

    // Early: max 5 ships, starts with 1
    assertTrue(sys.canAddShip("fleet1"), "Can add ship (1/5)");
    assertTrue(sys.addShipToFleet("fleet1"), "Add ship 2");
    assertTrue(sys.addShipToFleet("fleet1"), "Add ship 3");
    assertTrue(sys.addShipToFleet("fleet1"), "Add ship 4");
    assertTrue(sys.addShipToFleet("fleet1"), "Add ship 5");
    assertTrue(!sys.canAddShip("fleet1"), "Cannot add at capacity (5/5)");
    assertTrue(!sys.addShipToFleet("fleet1"), "addShip fails at capacity");
}

static void testFleetProgressionRemoveShip() {
    std::cout << "\n=== FleetProgression: RemoveShip ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 0.0f);

    // current_ship_count starts at 1
    sys.removeShipFromFleet("fleet1");
    // Now at 0; removing again should stay at 0
    sys.removeShipFromFleet("fleet1");
    assertTrue(!sys.canAddShip("fleet1") == false, "Can add ship after removal");
}

static void testFleetProgressionWingRoles() {
    std::cout << "\n=== FleetProgression: WingRoles ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 0.0f);

    // Early stage: no roles unlocked by default
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "mining"), "Mining not unlocked in Early");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "combat"), "Combat not unlocked in Early");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "logistics"), "Logistics not unlocked in Early");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage not unlocked in Early");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "construction"), "Construction not unlocked in Early");

    // Manual unlock
    sys.unlockWingRole("fleet1", "mining");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "mining"), "Mining manually unlocked");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "combat"), "Combat still locked");
}

static void testFleetProgressionAutoUnlockMid() {
    std::cout << "\n=== FleetProgression: AutoUnlockMid ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 100.0f);

    // updateComponent triggers auto-unlock at Mid
    sys.update(0.1f);

    assertTrue(sys.isWingRoleUnlocked("fleet1", "mining"), "Mining unlocked at Mid");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "combat"), "Combat unlocked at Mid");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "logistics"), "Logistics unlocked at Mid");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage NOT unlocked at Mid");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "construction"), "Construction NOT unlocked at Mid");
}

static void testFleetProgressionAutoUnlockEnd() {
    std::cout << "\n=== FleetProgression: AutoUnlockEnd ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 500.0f);

    sys.update(0.1f);

    assertTrue(sys.isWingRoleUnlocked("fleet1", "mining"), "Mining unlocked at End");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "combat"), "Combat unlocked at End");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "logistics"), "Logistics unlocked at End");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage unlocked at End");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "construction"), "Construction unlocked at End");
}

static void testFleetProgressionCapacityAfterStageUp() {
    std::cout << "\n=== FleetProgression: CapacityAfterStageUp ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 0.0f);

    // Fill to Early capacity (5)
    for (int i = 0; i < 4; i++) sys.addShipToFleet("fleet1");
    assertTrue(!sys.canAddShip("fleet1"), "Full at Early (5/5)");

    // Advance to Mid
    sys.addExperience("fleet1", 100.0f);
    assertTrue(sys.canAddShip("fleet1"), "Can add after Mid stage (5/15)");
    assertTrue(sys.getMaxShips("fleet1") == 15, "Max ships now 15");
}

static void testFleetProgressionUnknownRole() {
    std::cout << "\n=== FleetProgression: UnknownRole ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    world.createEntity("fleet1");
    sys.addExperience("fleet1", 0.0f);

    sys.unlockWingRole("fleet1", "scouting");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "scouting"), "Unknown role stays locked");
}

static void testFleetProgressionMissingEntity() {
    std::cout << "\n=== FleetProgression: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);

    assertTrue(sys.getStage("x") == components::FleetProgression::Stage::Early, "Stage default on missing");
    assertTrue(sys.getMaxShips("x") == 5, "MaxShips default on missing");
    assertTrue(sys.getMaxWings("x") == 1, "MaxWings default on missing");
    assertTrue(!sys.canAddShip("x"), "canAddShip false on missing");
    assertTrue(!sys.addShipToFleet("x"), "addShip fails on missing");
    assertTrue(!sys.isWingRoleUnlocked("x", "mining"), "Role check false on missing");
}

void run_fleet_progression_system_tests() {
    testFleetProgressionInitialState();
    testFleetProgressionMidStage();
    testFleetProgressionEndStage();
    testFleetProgressionAddShip();
    testFleetProgressionRemoveShip();
    testFleetProgressionWingRoles();
    testFleetProgressionAutoUnlockMid();
    testFleetProgressionAutoUnlockEnd();
    testFleetProgressionCapacityAfterStageUp();
    testFleetProgressionUnknownRole();
    testFleetProgressionMissingEntity();
}
