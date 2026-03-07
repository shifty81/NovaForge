// Tests for: Crew Management System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/crew_management_system.h"

using namespace atlas;

// ==================== Crew Management System Tests ====================

static void testCrewManagementCreate() {
    std::cout << "\n=== CrewManagement: Create ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 5), "Init succeeds");
    assertTrue(sys.getCrewCount("ship1") == 0, "No crew initially");
    assertTrue(sys.getAssignedCount("ship1") == 0, "None assigned");
    assertTrue(approxEqual(sys.getAverageMorale("ship1"), 0.5f), "Default morale 0.5");
    assertTrue(sys.getTotalSalaryPaid("ship1") == 0.0, "No salary paid");
    assertTrue(sys.getTotalHired("ship1") == 0, "0 hired");
    assertTrue(sys.getTotalDismissed("ship1") == 0, "0 dismissed");
    assertTrue(sys.getMoraleLevel("ship1") == "Normal", "Normal morale level");
}

static void testCrewManagementHire() {
    std::cout << "\n=== CrewManagement: Hire ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);

    assertTrue(sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f), "Hire Alice");
    assertTrue(sys.hireCrew("ship1", "Bob", "Engineer", 7, 120.0f), "Hire Bob");
    assertTrue(sys.getCrewCount("ship1") == 2, "2 crew members");
    assertTrue(sys.getTotalHired("ship1") == 2, "2 total hired");
}

static void testCrewManagementDuplicateHire() {
    std::cout << "\n=== CrewManagement: DuplicateHire ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);
    assertTrue(!sys.hireCrew("ship1", "Alice", "Gunner", 3, 80.0f), "Duplicate name rejected");
    assertTrue(sys.getCrewCount("ship1") == 1, "Still 1 crew member");
}

static void testCrewManagementMaxCrew() {
    std::cout << "\n=== CrewManagement: MaxCrew ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 2);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);
    sys.hireCrew("ship1", "Bob", "Engineer", 7, 120.0f);
    assertTrue(!sys.hireCrew("ship1", "Carol", "Gunner", 4, 90.0f), "Max crew enforced");
    assertTrue(sys.getCrewCount("ship1") == 2, "Still 2 crew");
}

static void testCrewManagementDismiss() {
    std::cout << "\n=== CrewManagement: Dismiss ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);
    sys.hireCrew("ship1", "Bob", "Engineer", 7, 120.0f);

    assertTrue(sys.dismissCrew("ship1", "Alice"), "Dismiss Alice");
    assertTrue(sys.getCrewCount("ship1") == 1, "1 crew remaining");
    assertTrue(sys.getTotalDismissed("ship1") == 1, "1 dismissed");
    assertTrue(!sys.dismissCrew("ship1", "Alice"), "Cannot dismiss again");
}

static void testCrewManagementAssignUnassign() {
    std::cout << "\n=== CrewManagement: AssignUnassign ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);
    sys.hireCrew("ship1", "Bob", "Engineer", 7, 120.0f);

    assertTrue(sys.assignCrew("ship1", "Alice"), "Assign Alice");
    assertTrue(sys.getAssignedCount("ship1") == 1, "1 assigned");
    assertTrue(!sys.assignCrew("ship1", "Alice"), "Cannot double-assign");

    assertTrue(sys.assignCrew("ship1", "Bob"), "Assign Bob");
    assertTrue(sys.getAssignedCount("ship1") == 2, "2 assigned");

    assertTrue(sys.unassignCrew("ship1", "Bob"), "Unassign Bob");
    assertTrue(sys.getAssignedCount("ship1") == 1, "1 assigned");
    assertTrue(!sys.unassignCrew("ship1", "Bob"), "Cannot unassign already unassigned");
}

static void testCrewManagementMorale() {
    std::cout << "\n=== CrewManagement: Morale ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);

    // Initial morale is 0.5 → Normal
    assertTrue(approxEqual(sys.getAverageMorale("ship1"), 0.5f), "Initial morale 0.5");
    assertTrue(sys.getMoraleLevel("ship1") == "Normal", "Normal level");

    // Boost morale
    assertTrue(sys.adjustMorale("ship1", "Alice", 0.4f), "Boost morale by 0.4");
    assertTrue(approxEqual(sys.getAverageMorale("ship1"), 0.9f), "Morale now 0.9");
    assertTrue(sys.getMoraleLevel("ship1") == "Exceptional", "Exceptional level");

    // Drop morale
    assertTrue(sys.adjustMorale("ship1", "Alice", -0.8f), "Drop morale by 0.8");
    assertTrue(approxEqual(sys.getAverageMorale("ship1"), 0.1f), "Morale now 0.1");
    assertTrue(sys.getMoraleLevel("ship1") == "Mutinous", "Mutinous level");
}

static void testCrewManagementMoraleClamping() {
    std::cout << "\n=== CrewManagement: MoraleClamping ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);

    sys.adjustMorale("ship1", "Alice", 10.0f);
    assertTrue(approxEqual(sys.getAverageMorale("ship1"), 1.0f), "Clamped at 1.0");

    sys.adjustMorale("ship1", "Alice", -20.0f);
    assertTrue(approxEqual(sys.getAverageMorale("ship1"), 0.0f), "Clamped at 0.0");
}

static void testCrewManagementEfficiency() {
    std::cout << "\n=== CrewManagement: Efficiency ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 10, 100.0f); // max skill
    sys.assignCrew("ship1", "Alice");

    // morale 0.5 → morale_factor=1.0, skill 10/10=1.0 → skill_factor=1.0
    // efficiency = 1.0 * (0.5 + 1.0) = 1.5
    assertTrue(sys.getEfficiencyMultiplier("ship1") > 1.0f, "High-skill assigned crew boosts efficiency");

    sys.adjustMorale("ship1", "Alice", 0.45f); // morale → 0.95
    // morale_factor = 0.5 + 0.95 = 1.45, skill_factor = 1.0
    // efficiency = 1.45 * (0.5 + 1.0) = 2.175
    assertTrue(sys.getEfficiencyMultiplier("ship1") > 1.5f, "High morale + skill = high efficiency");
}

static void testCrewManagementSalaryPayment() {
    std::cout << "\n=== CrewManagement: SalaryPayment ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 5);
    sys.hireCrew("ship1", "Alice", "Pilot", 5, 100.0f);
    sys.assignCrew("ship1", "Alice");

    auto* entity = world.getEntity("ship1");
    auto* comp = entity->getComponent<components::CrewManagement>();
    comp->salary_interval = 10.0f; // pay every 10 seconds for test

    sys.update(5.0f); // half interval, no payment yet
    assertTrue(sys.getTotalSalaryPaid("ship1") == 0.0, "No payment before interval");

    sys.update(6.0f); // 11s total, past 10s interval
    assertTrue(sys.getTotalSalaryPaid("ship1") > 0.0, "Salary paid after interval");
}

static void testCrewManagementMissing() {
    std::cout << "\n=== CrewManagement: Missing ===" << std::endl;
    ecs::World world;
    systems::CrewManagementSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 5), "Init fails on missing");
    assertTrue(!sys.hireCrew("nonexistent", "Alice", "Pilot", 5, 100.0f), "Hire fails on missing");
    assertTrue(!sys.dismissCrew("nonexistent", "Alice"), "Dismiss fails on missing");
    assertTrue(!sys.assignCrew("nonexistent", "Alice"), "Assign fails on missing");
    assertTrue(!sys.unassignCrew("nonexistent", "Alice"), "Unassign fails on missing");
    assertTrue(!sys.adjustMorale("nonexistent", "Alice", 0.1f), "AdjustMorale fails on missing");
    assertTrue(sys.getCrewCount("nonexistent") == 0, "0 crew on missing");
    assertTrue(sys.getAssignedCount("nonexistent") == 0, "0 assigned on missing");
    assertTrue(sys.getAverageMorale("nonexistent") == 0.0f, "0 morale on missing");
    assertTrue(sys.getEfficiencyMultiplier("nonexistent") == 0.0f, "0 efficiency on missing");
    assertTrue(sys.getTotalSalaryPaid("nonexistent") == 0.0, "0 salary on missing");
    assertTrue(sys.getTotalHired("nonexistent") == 0, "0 hired on missing");
    assertTrue(sys.getTotalDismissed("nonexistent") == 0, "0 dismissed on missing");
    assertTrue(sys.getMoraleLevel("nonexistent") == "Unknown", "Unknown morale on missing");
}


void run_crew_management_system_tests() {
    testCrewManagementCreate();
    testCrewManagementHire();
    testCrewManagementDuplicateHire();
    testCrewManagementMaxCrew();
    testCrewManagementDismiss();
    testCrewManagementAssignUnassign();
    testCrewManagementMorale();
    testCrewManagementMoraleClamping();
    testCrewManagementEfficiency();
    testCrewManagementSalaryPayment();
    testCrewManagementMissing();
}
