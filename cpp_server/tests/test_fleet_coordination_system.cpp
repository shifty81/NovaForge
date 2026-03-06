// Tests for: Fleet Coordination System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_coordination_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Fleet Coordination System Tests ====================

static void testFleetCoordinationCreate() {
    std::cout << "\n=== FleetCoordination: Create ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    assertTrue(sys.initialize("fc1", "fleet_alpha"), "Init succeeds");
    assertTrue(sys.getOrder("fc1") == 0, "Default order is Hold");
    assertTrue(sys.getTargetCount("fc1") == 0, "No targets initially");
    assertTrue(sys.getShipCount("fc1") == 0, "No ships initially");
    assertTrue(approxEqual(sys.getCombatReadiness("fc1"), 1.0f), "Full readiness");
    assertTrue(!sys.isInCombat("fc1"), "Not in combat initially");
}

static void testFleetCoordinationIssueOrder() {
    std::cout << "\n=== FleetCoordination: IssueOrder ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    assertTrue(sys.issueOrder("fc1", 1), "Issue Engage order");
    assertTrue(sys.getOrder("fc1") == 1, "Order is Engage");
    assertTrue(sys.issueOrder("fc1", 2), "Issue FocusFire order");
    assertTrue(sys.getOrder("fc1") == 2, "Order is FocusFire");
    assertTrue(sys.getTotalOrdersIssued("fc1") == 2, "2 orders issued");
    assertTrue(!sys.issueOrder("fc1", 6), "Invalid order rejected");
    assertTrue(!sys.issueOrder("fc1", -1), "Negative order rejected");
}

static void testFleetCoordinationAssignTarget() {
    std::cout << "\n=== FleetCoordination: AssignTarget ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    assertTrue(sys.assignTarget("fc1", "enemy1", 3), "Assign target priority 3");
    assertTrue(sys.assignTarget("fc1", "enemy2", 5), "Assign target priority 5");
    assertTrue(sys.getTargetCount("fc1") == 2, "2 targets");
    assertTrue(!sys.assignTarget("fc1", "enemy1", 4), "Duplicate target rejected");
}

static void testFleetCoordinationRemoveTarget() {
    std::cout << "\n=== FleetCoordination: RemoveTarget ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    sys.assignTarget("fc1", "enemy1", 3);
    assertTrue(sys.removeTarget("fc1", "enemy1"), "Remove target succeeds");
    assertTrue(sys.getTargetCount("fc1") == 0, "0 targets after remove");
    assertTrue(!sys.removeTarget("fc1", "enemy1"), "Double remove fails");
}

static void testFleetCoordinationAddRemoveShip() {
    std::cout << "\n=== FleetCoordination: AddRemoveShip ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    assertTrue(sys.addShip("fc1", "ship1"), "Add ship 1");
    assertTrue(sys.addShip("fc1", "ship2"), "Add ship 2");
    assertTrue(sys.getShipCount("fc1") == 2, "2 ships");
    assertTrue(!sys.addShip("fc1", "ship1"), "Duplicate ship rejected");
    assertTrue(sys.removeShip("fc1", "ship1"), "Remove ship 1");
    assertTrue(sys.getShipCount("fc1") == 1, "1 ship after remove");
    assertTrue(!sys.removeShip("fc1", "ship1"), "Double remove fails");
}

static void testFleetCoordinationCombatState() {
    std::cout << "\n=== FleetCoordination: CombatState ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    assertTrue(!sys.isInCombat("fc1"), "Not in combat");
    assertTrue(sys.enterCombat("fc1"), "Enter combat");
    assertTrue(sys.isInCombat("fc1"), "In combat");
    assertTrue(sys.leaveCombat("fc1"), "Leave combat");
    assertTrue(!sys.isInCombat("fc1"), "Not in combat after leave");
}

static void testFleetCoordinationCombatReadiness() {
    std::cout << "\n=== FleetCoordination: CombatReadiness ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    assertTrue(approxEqual(sys.getCombatReadiness("fc1"), 1.0f), "Full readiness at start");
    sys.setFormationCoherence("fc1", 0.8f);
    sys.setMoraleFactor("fc1", 0.5f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatReadiness("fc1"), 0.4f), "Readiness = 0.8 × 0.5 = 0.4");
}

static void testFleetCoordinationCoherenceDecay() {
    std::cout << "\n=== FleetCoordination: CoherenceDecay ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    sys.enterCombat("fc1");

    auto* entity = world.getEntity("fc1");
    auto* fc = entity->getComponent<components::FleetCoordination>();
    fc->coherence_decay_rate = 0.1f;

    float before = fc->formation_coherence;
    sys.update(2.0f);
    assertTrue(fc->formation_coherence < before, "Coherence decayed in combat");
    assertTrue(approxEqual(fc->formation_coherence, 0.8f), "Coherence = 1.0 - 0.1*2 = 0.8");
}

static void testFleetCoordinationHighestPriorityTarget() {
    std::cout << "\n=== FleetCoordination: HighestPriorityTarget ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");
    sys.assignTarget("fc1", "enemy1", 2);
    sys.assignTarget("fc1", "enemy2", 5);
    sys.assignTarget("fc1", "enemy3", 3);
    assertTrue(sys.getHighestPriorityTarget("fc1") == "enemy2", "Enemy2 is highest priority");
}

static void testFleetCoordinationMaxLimits() {
    std::cout << "\n=== FleetCoordination: MaxLimits ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    world.createEntity("fc1");
    sys.initialize("fc1", "fleet_alpha");

    auto* entity = world.getEntity("fc1");
    auto* fc = entity->getComponent<components::FleetCoordination>();
    fc->max_targets = 2;
    fc->max_ships = 2;

    sys.assignTarget("fc1", "e1", 1);
    sys.assignTarget("fc1", "e2", 2);
    assertTrue(!sys.assignTarget("fc1", "e3", 3), "Max targets enforced");

    sys.addShip("fc1", "s1");
    sys.addShip("fc1", "s2");
    assertTrue(!sys.addShip("fc1", "s3"), "Max ships enforced");
}

static void testFleetCoordinationMissing() {
    std::cout << "\n=== FleetCoordination: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetCoordinationSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "fleet1"), "Init fails on missing");
    assertTrue(!sys.issueOrder("nonexistent", 1), "IssueOrder fails on missing");
    assertTrue(!sys.assignTarget("nonexistent", "e1", 3), "AssignTarget fails on missing");
    assertTrue(!sys.removeTarget("nonexistent", "e1"), "RemoveTarget fails on missing");
    assertTrue(!sys.addShip("nonexistent", "s1"), "AddShip fails on missing");
    assertTrue(!sys.removeShip("nonexistent", "s1"), "RemoveShip fails on missing");
    assertTrue(!sys.enterCombat("nonexistent"), "EnterCombat fails on missing");
    assertTrue(!sys.leaveCombat("nonexistent"), "LeaveCombat fails on missing");
    assertTrue(sys.getOrder("nonexistent") == 0, "0 order on missing");
    assertTrue(sys.getTargetCount("nonexistent") == 0, "0 targets on missing");
    assertTrue(sys.getShipCount("nonexistent") == 0, "0 ships on missing");
    assertTrue(approxEqual(sys.getCombatReadiness("nonexistent"), 0.0f), "0 readiness on missing");
    assertTrue(sys.getHighestPriorityTarget("nonexistent") == "", "Empty target on missing");
    assertTrue(!sys.isInCombat("nonexistent"), "Not in combat on missing");
    assertTrue(sys.getTotalOrdersIssued("nonexistent") == 0, "0 orders on missing");
}


void run_fleet_coordination_system_tests() {
    testFleetCoordinationCreate();
    testFleetCoordinationIssueOrder();
    testFleetCoordinationAssignTarget();
    testFleetCoordinationRemoveTarget();
    testFleetCoordinationAddRemoveShip();
    testFleetCoordinationCombatState();
    testFleetCoordinationCombatReadiness();
    testFleetCoordinationCoherenceDecay();
    testFleetCoordinationHighestPriorityTarget();
    testFleetCoordinationMaxLimits();
    testFleetCoordinationMissing();
}
