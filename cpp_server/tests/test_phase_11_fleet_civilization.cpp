// Tests for: Phase 11: Fleet Civilization Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"

using namespace atlas;

// ==================== Phase 11: Fleet Civilization Tests ====================

static void testFleetCivilizationThresholdNotMet() {
    std::cout << "\n=== Fleet Civilization Threshold Not Met ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    auto* civ = addComp<components::FleetCivilization>(fleet);
    assertTrue(!civ->isCivilizationThresholdMet(), "Threshold not met initially");
}

static void testFleetCivilizationThresholdMet() {
    std::cout << "\n=== Fleet Civilization Threshold Met ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    auto* civ = addComp<components::FleetCivilization>(fleet);
    civ->has_stable_logistics = true;
    civ->has_loyal_captains = true;
    civ->has_fleet_history = true;
    civ->has_fleet_industry = true;
    assertTrue(civ->isCivilizationThresholdMet(), "Threshold met when all criteria true");
}


void run_phase_11_fleet_civilization_tests() {
    testFleetCivilizationThresholdNotMet();
    testFleetCivilizationThresholdMet();
}
