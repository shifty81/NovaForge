// Tests for: Crew System Tests
#include "test_log.h"
#include "components/crew_components.h"
#include "ecs/system.h"
#include "systems/crew_system.h"

using namespace atlas;

// ==================== Crew System Tests ====================

static void testCrewDefaults() {
    std::cout << "\n=== Crew Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* crew = addComp<components::ShipCrew>(e);
    assertTrue(crew->current_crew == 0, "No crew initially");
    assertTrue(crew->crew_member_ids.empty(), "Empty crew list");
}

static void testCrewAssign() {
    std::cout << "\n=== Crew Assign ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("ship2");
    addComp<components::ShipCrew>(ship);
    auto* member = world.createEntity("crew_a");
    addComp<components::CrewMember>(member);

    systems::CrewSystem sys(&world);
    bool result = sys.assignCrew("ship2", "crew_a", "engine_room");
    assertTrue(result, "Assign succeeded");
    assertTrue(sys.getCrewCount("ship2") == 1, "Crew count is 1");
}

static void testCrewEfficiency() {
    std::cout << "\n=== Crew Efficiency ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("ship3");
    addComp<components::ShipCrew>(ship);
    auto* m1 = world.createEntity("crew_b");
    auto* cm = addComp<components::CrewMember>(m1);
    cm->skill_level = 3.0f;
    cm->morale = 80.0f;

    systems::CrewSystem sys(&world);
    sys.assignCrew("ship3", "crew_b", "bridge");
    sys.update(0.0f);

    // efficiency_bonus = (3.0 * 0.1) + (80.0 / 200.0) = 0.3 + 0.4 = 0.7
    assertTrue(approxEqual(cm->efficiency_bonus, 0.7f), "Efficiency bonus is 0.7");
    assertTrue(approxEqual(sys.getOverallEfficiency("ship3"), 0.7f), "Overall efficiency is 0.7");
}

static void testCrewRemove() {
    std::cout << "\n=== Crew Remove ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("ship4");
    addComp<components::ShipCrew>(ship);
    auto* m1 = world.createEntity("crew_c");
    addComp<components::CrewMember>(m1);

    systems::CrewSystem sys(&world);
    sys.assignCrew("ship4", "crew_c", "medbay");
    assertTrue(sys.getCrewCount("ship4") == 1, "Crew count 1 after assign");
    bool result = sys.removeCrew("ship4", "crew_c");
    assertTrue(result, "Remove succeeded");
    assertTrue(sys.getCrewCount("ship4") == 0, "Crew count 0 after remove");
}


void run_crew_system_tests() {
    testCrewDefaults();
    testCrewAssign();
    testCrewEfficiency();
    testCrewRemove();
}
