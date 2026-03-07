// Tests for: Formation Flight System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/formation_flight_system.h"

using namespace atlas;

// ==================== Formation Flight System Tests ====================

static void testFormationFlightCreate() {
    std::cout << "\n=== FormationFlight: Create ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    assertTrue(sys.initialize("wing1", "fleet_alpha", "leader_01", 1), "Init succeeds");
    assertTrue(sys.getFormationType("wing1") == "Wedge", "Default Wedge formation");
    assertTrue(sys.getSlotStatus("wing1") == "Holding", "Holding initially");
    assertTrue(approxEqual(sys.getCohesion("wing1"), 1.0f), "Full cohesion");
    assertTrue(sys.getCohesionBonus("wing1") == 0.0f, "No bonus before first update");
    assertTrue(sys.getFormationBreaks("wing1") == 0, "No breaks");
    assertTrue(sys.getFormationReforms("wing1") == 0, "No reforms");
    assertTrue(sys.getDriftDistance("wing1") == 0.0, "No drift");
}

static void testFormationFlightSetType() {
    std::cout << "\n=== FormationFlight: SetType ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    sys.initialize("wing1", "fleet_alpha", "leader_01", 1);

    assertTrue(sys.setFormationType("wing1", "Line"), "Set Line");
    assertTrue(sys.getFormationType("wing1") == "Line", "Formation is Line");

    assertTrue(sys.setFormationType("wing1", "Sphere"), "Set Sphere");
    assertTrue(sys.getFormationType("wing1") == "Sphere", "Formation is Sphere");

    assertTrue(sys.setFormationType("wing1", "Wall"), "Set Wall");
    assertTrue(sys.getFormationType("wing1") == "Wall", "Formation is Wall");

    assertTrue(sys.setFormationType("wing1", "Echelon"), "Set Echelon");
    assertTrue(sys.getFormationType("wing1") == "Echelon", "Formation is Echelon");
}

static void testFormationFlightSlotOffset() {
    std::cout << "\n=== FormationFlight: SlotOffset ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    sys.initialize("wing1", "fleet_alpha", "leader_01", 2);

    assertTrue(sys.setSlotOffset("wing1", 10.0, 0.0, -5.0), "Set slot offset");
    // Actual position at origin, offset at (10,0,-5) -> drift = sqrt(125) ~ 11.18
    assertTrue(sys.getDriftDistance("wing1") > 10.0, "Drift distance reflects offset");
}

static void testFormationFlightCohesionDrift() {
    std::cout << "\n=== FormationFlight: CohesionDrift ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    sys.initialize("wing1", "fleet_alpha", "leader_01", 1);
    sys.setMaxDrift("wing1", 100.0f);
    sys.setSlotOffset("wing1", 50.0, 0.0, 0.0);

    // Actual at origin, offset at (50,0,0) -> drift=50, cohesion = 1 - 50/100 = 0.5
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getCohesion("wing1"), 0.5f), "50% cohesion at half drift");
    assertTrue(sys.getSlotStatus("wing1") == "Drifting", "Drifting at 0.5 cohesion");

    // Move actual closer to offset
    sys.updateActualPosition("wing1", 40.0, 0.0, 0.0);
    // drift = 10, cohesion = 1 - 10/100 = 0.9
    assertTrue(approxEqual(sys.getCohesion("wing1"), 0.9f), "90% cohesion near target");
    assertTrue(sys.getSlotStatus("wing1") == "Holding", "Holding at 0.9 cohesion");
}

static void testFormationFlightBrokenFormation() {
    std::cout << "\n=== FormationFlight: BrokenFormation ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    sys.initialize("wing1", "fleet_alpha", "leader_01", 1);
    sys.setMaxDrift("wing1", 50.0f);
    sys.setSlotOffset("wing1", 0.0, 0.0, 0.0);

    // Move far from offset
    sys.updateActualPosition("wing1", 100.0, 0.0, 0.0);
    // drift = 100, max = 50 -> cohesion = max(0, 1 - 100/50) = 0
    sys.update(1.0f);
    assertTrue(sys.getSlotStatus("wing1") == "Broken", "Broken when drift exceeds max");
    assertTrue(sys.getFormationBreaks("wing1") == 1, "1 break recorded");
    assertTrue(sys.getCohesion("wing1") == 0.0f, "Zero cohesion");
    assertTrue(sys.getCohesionBonus("wing1") == 0.0f, "No bonus when broken");
}

static void testFormationFlightReform() {
    std::cout << "\n=== FormationFlight: Reform ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    sys.initialize("wing1", "fleet_alpha", "leader_01", 1);
    sys.setMaxDrift("wing1", 50.0f);
    sys.setSlotOffset("wing1", 0.0, 0.0, 0.0);

    // Break formation
    sys.updateActualPosition("wing1", 100.0, 0.0, 0.0);
    sys.update(1.0f);
    assertTrue(sys.getSlotStatus("wing1") == "Broken", "Formation broken");

    // Start reforming
    assertTrue(sys.reformFormation("wing1"), "Reform command");
    assertTrue(sys.getSlotStatus("wing1") == "Reforming", "Status is Reforming");

    // Move back within threshold
    sys.updateActualPosition("wing1", 5.0, 0.0, 0.0);
    // drift = 5, cohesion = 1 - 5/50 = 0.9 -> >= 0.8 threshold
    sys.update(1.0f);
    assertTrue(sys.getSlotStatus("wing1") == "Holding", "Holding after reform");
    assertTrue(sys.getFormationReforms("wing1") == 1, "1 reform recorded");
}

static void testFormationFlightMultipleBreaksReforms() {
    std::cout << "\n=== FormationFlight: MultipleBreaksReforms ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    world.createEntity("wing1");
    sys.initialize("wing1", "fleet_alpha", "leader_01", 1);
    sys.setMaxDrift("wing1", 50.0f);
    sys.setSlotOffset("wing1", 0.0, 0.0, 0.0);

    // First break
    sys.updateActualPosition("wing1", 100.0, 0.0, 0.0);
    sys.update(1.0f);
    assertTrue(sys.getFormationBreaks("wing1") == 1, "First break");

    // Reform
    sys.reformFormation("wing1");
    sys.updateActualPosition("wing1", 2.0, 0.0, 0.0);
    sys.update(1.0f);
    assertTrue(sys.getFormationReforms("wing1") == 1, "First reform");

    // Second break
    sys.updateActualPosition("wing1", 200.0, 0.0, 0.0);
    sys.update(1.0f);
    assertTrue(sys.getFormationBreaks("wing1") == 2, "Second break");
}

static void testFormationFlightMissing() {
    std::cout << "\n=== FormationFlight: Missing ===" << std::endl;
    ecs::World world;
    systems::FormationFlightSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "fleet", "leader", 0), "Init fails on missing");
    assertTrue(!sys.setFormationType("nonexistent", "Line"), "SetFormationType fails");
    assertTrue(!sys.setSlotOffset("nonexistent", 0, 0, 0), "SetSlotOffset fails");
    assertTrue(!sys.updateActualPosition("nonexistent", 0, 0, 0), "UpdateActualPosition fails");
    assertTrue(!sys.setMaxDrift("nonexistent", 50.0f), "SetMaxDrift fails");
    assertTrue(!sys.reformFormation("nonexistent"), "ReformFormation fails");
    assertTrue(sys.getFormationType("nonexistent") == "Unknown", "Unknown formation");
    assertTrue(sys.getSlotStatus("nonexistent") == "Unknown", "Unknown status");
    assertTrue(sys.getCohesion("nonexistent") == 0.0f, "0 cohesion on missing");
    assertTrue(sys.getCohesionBonus("nonexistent") == 0.0f, "0 bonus on missing");
    assertTrue(sys.getFormationBreaks("nonexistent") == 0, "0 breaks on missing");
    assertTrue(sys.getFormationReforms("nonexistent") == 0, "0 reforms on missing");
    assertTrue(sys.getDriftDistance("nonexistent") == 0.0, "0 drift on missing");
}


void run_formation_flight_system_tests() {
    testFormationFlightCreate();
    testFormationFlightSetType();
    testFormationFlightSlotOffset();
    testFormationFlightCohesionDrift();
    testFormationFlightBrokenFormation();
    testFormationFlightReform();
    testFormationFlightMultipleBreaksReforms();
    testFormationFlightMissing();
}
