// Tests for: FPSObjectiveSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "components/mission_components.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fps_objective_system.h"
#include "systems/movement_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== FPSObjectiveSystem Tests ====================

static void testFPSObjectiveCreate() {
    std::cout << "\n=== FPS Objective Create ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    bool ok = sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                                   components::FPSObjective::ObjectiveType::EliminateHostiles,
                                   "Clear the bridge of enemies");
    assertTrue(ok, "Create objective succeeds");
    assertTrue(!sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                                     components::FPSObjective::ObjectiveType::EliminateHostiles),
               "Duplicate create fails");
}

static void testFPSObjectiveActivate() {
    std::cout << "\n=== FPS Objective Activate ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    assertTrue(sys.getObjectiveState("obj_1") ==
               static_cast<int>(components::FPSObjective::ObjectiveState::Inactive),
               "Starts inactive");

    assertTrue(sys.activateObjective("obj_1"), "Activate succeeds");
    assertTrue(sys.getObjectiveState("obj_1") ==
               static_cast<int>(components::FPSObjective::ObjectiveState::Active),
               "Now active");

    // Can't activate again
    assertTrue(!sys.activateObjective("obj_1"), "Double activate fails");
}

static void testFPSObjectiveEliminateHostiles() {
    std::cout << "\n=== FPS Objective Eliminate Hostiles ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    sys.setHostileCount("obj_1", 3);
    sys.activateObjective("obj_1");

    sys.reportHostileKill("obj_1");
    assertTrue(approxEqual(sys.getProgress("obj_1"), 1.0f / 3.0f), "1/3 progress");

    sys.reportHostileKill("obj_1");
    assertTrue(approxEqual(sys.getProgress("obj_1"), 2.0f / 3.0f), "2/3 progress");

    sys.reportHostileKill("obj_1");
    assertTrue(sys.isComplete("obj_1"), "Objective complete after 3 kills");
    assertTrue(approxEqual(sys.getProgress("obj_1"), 1.0f), "Progress at 1.0");
}

static void testFPSObjectiveRetrieveItem() {
    std::cout << "\n=== FPS Objective Retrieve Item ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_cargo", "player1",
                         components::FPSObjective::ObjectiveType::RetrieveItem);
    sys.setTargetItem("obj_1", "data_core_alpha");
    sys.activateObjective("obj_1");

    // Wrong item
    assertTrue(!sys.reportItemCollected("obj_1", "wrong_item"), "Wrong item fails");
    assertTrue(!sys.isComplete("obj_1"), "Not complete yet");

    // Correct item
    assertTrue(sys.reportItemCollected("obj_1", "data_core_alpha"), "Correct item succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

static void testFPSObjectiveDefendPoint() {
    std::cout << "\n=== FPS Objective Defend Point ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_eng", "player1",
                         components::FPSObjective::ObjectiveType::DefendPoint);
    sys.setDefendDuration("obj_1", 60.0f);
    sys.activateObjective("obj_1");

    sys.update(30.0f);
    assertTrue(approxEqual(sys.getProgress("obj_1"), 0.5f), "50% progress at 30s");
    assertTrue(!sys.isComplete("obj_1"), "Not complete at 30s");

    sys.update(30.0f);
    assertTrue(sys.isComplete("obj_1"), "Complete after 60s");
}

static void testFPSObjectiveSabotage() {
    std::cout << "\n=== FPS Objective Sabotage ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_eng", "player1",
                         components::FPSObjective::ObjectiveType::Sabotage,
                         "Destroy the reactor core");
    sys.activateObjective("obj_1");

    assertTrue(sys.reportSabotageComplete("obj_1"), "Sabotage report succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

static void testFPSObjectiveEscape() {
    std::cout << "\n=== FPS Objective Escape ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_airlock", "player1",
                         components::FPSObjective::ObjectiveType::Escape);
    sys.activateObjective("obj_1");

    assertTrue(sys.reportExtraction("obj_1"), "Extraction succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

static void testFPSObjectiveRescueVIP() {
    std::cout << "\n=== FPS Objective Rescue VIP ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_quarters", "player1",
                         components::FPSObjective::ObjectiveType::RescueVIP,
                         "Rescue the ambassador");
    sys.activateObjective("obj_1");

    assertTrue(sys.reportVIPRescued("obj_1"), "VIP rescue succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

static void testFPSObjectiveRepairSystem() {
    std::cout << "\n=== FPS Objective Repair System ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_eng", "player1",
                         components::FPSObjective::ObjectiveType::RepairSystem,
                         "Fix the reactor coolant system");
    sys.activateObjective("obj_1");

    assertTrue(sys.reportRepairComplete("obj_1"), "Repair report succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

static void testFPSObjectiveTimeLimit() {
    std::cout << "\n=== FPS Objective Time Limit ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles,
                         "Clear the bridge", 10.0f);
    sys.setHostileCount("obj_1", 5);
    sys.activateObjective("obj_1");

    sys.update(10.0f);
    assertTrue(sys.isFailed("obj_1"), "Objective failed due to time limit");
}

static void testFPSObjectiveFailManual() {
    std::cout << "\n=== FPS Objective Manual Fail ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::DefendPoint);
    sys.activateObjective("obj_1");

    assertTrue(sys.failObjective("obj_1"), "Manual fail succeeds");
    assertTrue(sys.isFailed("obj_1"), "Objective is failed");
}

static void testFPSObjectivePlayerList() {
    std::cout << "\n=== FPS Objective Player List ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_a", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    sys.createObjective("obj_2", "interior_1", "room_b", "player1",
                         components::FPSObjective::ObjectiveType::Sabotage);
    sys.createObjective("obj_3", "interior_1", "room_c", "player2",
                         components::FPSObjective::ObjectiveType::Escape);

    sys.activateObjective("obj_1");
    sys.activateObjective("obj_2");
    sys.activateObjective("obj_3");

    auto p1_objs = sys.getPlayerObjectives("player1");
    assertTrue(p1_objs.size() == 2, "Player1 has 2 objectives");

    auto p2_objs = sys.getPlayerObjectives("player2");
    assertTrue(p2_objs.size() == 1, "Player2 has 1 objective");
}

static void testFPSObjectiveWrongType() {
    std::cout << "\n=== FPS Objective Wrong Type Report ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_a", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    sys.activateObjective("obj_1");

    // Can't report sabotage on an eliminate objective
    assertTrue(!sys.reportSabotageComplete("obj_1"), "Wrong type report fails");
    assertTrue(!sys.reportExtraction("obj_1"), "Wrong type extraction fails");
    assertTrue(!sys.isComplete("obj_1"), "Not complete from wrong reports");
}

static void testFPSObjectiveTypeName() {
    std::cout << "\n=== FPS Objective Type Names ===" << std::endl;
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(0) == "EliminateHostiles", "EliminateHostiles name");
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(1) == "RescueVIP", "RescueVIP name");
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(2) == "Sabotage", "Sabotage name");
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(3) == "DefendPoint", "DefendPoint name");
    assertTrue(systems::FPSObjectiveSystem::stateName(0) == "Inactive", "Inactive state name");
    assertTrue(systems::FPSObjectiveSystem::stateName(2) == "Completed", "Completed state name");
}

static void testFPSObjectiveComponentDefaults() {
    std::cout << "\n=== FPS Objective Component Defaults ===" << std::endl;
    components::FPSObjective obj;
    assertTrue(obj.state == 0, "Default state inactive");
    assertTrue(obj.objective_type == 0, "Default type eliminate");
    assertTrue(approxEqual(obj.progress, 0.0f), "Default progress 0");
    assertTrue(approxEqual(obj.time_limit, 0.0f), "Default no time limit");
    assertTrue(obj.hostiles_required == 0, "Default 0 hostiles required");
    assertTrue(obj.hostiles_killed == 0, "Default 0 hostiles killed");
    assertTrue(!obj.item_collected, "Default item not collected");
}


void run_fpsobjective_system_tests() {
    testFPSObjectiveCreate();
    testFPSObjectiveActivate();
    testFPSObjectiveEliminateHostiles();
    testFPSObjectiveRetrieveItem();
    testFPSObjectiveDefendPoint();
    testFPSObjectiveSabotage();
    testFPSObjectiveEscape();
    testFPSObjectiveRescueVIP();
    testFPSObjectiveRepairSystem();
    testFPSObjectiveTimeLimit();
    testFPSObjectiveFailManual();
    testFPSObjectivePlayerList();
    testFPSObjectiveWrongType();
    testFPSObjectiveTypeName();
    testFPSObjectiveComponentDefaults();
}
