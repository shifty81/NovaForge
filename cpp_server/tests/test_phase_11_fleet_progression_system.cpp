// Tests for: Phase 11: Fleet Progression System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_progression_system.h"

using namespace atlas;

// ==================== Phase 11: Fleet Progression System Tests ====================

static void testFleetProgressionEarlyStage() {
    std::cout << "\n=== Fleet Progression Early Stage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Early,
               "Fleet starts in Early stage");
    assertTrue(sys.getMaxShips("fleet1") == 5, "Early stage max ships is 5");
    assertTrue(sys.getMaxWings("fleet1") == 1, "Early stage max wings is 1");
}

static void testFleetProgressionMidStage() {
    std::cout << "\n=== Fleet Progression Mid Stage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    sys.addExperience("fleet1", 150.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Mid,
               "Fleet advances to Mid stage at 100+ XP");
    assertTrue(sys.getMaxShips("fleet1") == 15, "Mid stage max ships is 15");
    assertTrue(sys.getMaxWings("fleet1") == 3, "Mid stage max wings is 3");
}

static void testFleetProgressionEndStage() {
    std::cout << "\n=== Fleet Progression End Stage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    sys.addExperience("fleet1", 600.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::End,
               "Fleet advances to End stage at 500+ XP");
    assertTrue(sys.getMaxShips("fleet1") == 25, "End stage max ships is 25");
    assertTrue(sys.getMaxWings("fleet1") == 5, "End stage max wings is 5");
}

static void testFleetProgressionCanAddShip() {
    std::cout << "\n=== Fleet Progression Can Add Ship ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    auto* prog = addComp<components::FleetProgression>(fleet);
    prog->current_ship_count = 4;
    assertTrue(sys.canAddShip("fleet1"), "Can add ship when below max");
    assertTrue(sys.addShipToFleet("fleet1"), "Successfully added ship");
    assertTrue(!sys.canAddShip("fleet1"), "Cannot add ship at max (5)");
}

static void testFleetProgressionRemoveShip() {
    std::cout << "\n=== Fleet Progression Remove Ship ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    auto* prog = addComp<components::FleetProgression>(fleet);
    prog->current_ship_count = 3;
    sys.removeShipFromFleet("fleet1");
    assertTrue(prog->current_ship_count == 2, "Ship count decremented");
}

static void testFleetProgressionWingRoleUnlock() {
    std::cout << "\n=== Fleet Progression Wing Role Unlock ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "mining"), "Mining wing locked in Early");
    sys.addExperience("fleet1", 150.0f);
    sys.update(0.0f);
    assertTrue(sys.isWingRoleUnlocked("fleet1", "mining"), "Mining wing unlocked in Mid");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "combat"), "Combat wing unlocked in Mid");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "logistics"), "Logistics wing unlocked in Mid");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage wing locked in Mid");
}

static void testFleetProgressionEndStageAllRoles() {
    std::cout << "\n=== Fleet Progression End Stage All Roles ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    sys.addExperience("fleet1", 600.0f);
    sys.update(0.0f);
    assertTrue(sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage wing unlocked in End");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "construction"), "Construction wing unlocked in End");
}


void run_phase_11_fleet_progression_system_tests() {
    testFleetProgressionEarlyStage();
    testFleetProgressionMidStage();
    testFleetProgressionEndStage();
    testFleetProgressionCanAddShip();
    testFleetProgressionRemoveShip();
    testFleetProgressionWingRoleUnlock();
    testFleetProgressionEndStageAllRoles();
}
