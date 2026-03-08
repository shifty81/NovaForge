// Tests for: FPSObjectiveSystem Tests
#include "test_log.h"
#include "components/game_components.h"
#include "systems/fps_objective_system.h"

using namespace atlas;
using FPSObjType = components::FPSObjective::ObjectiveType;
using FPSObjState = components::FPSObjective::ObjectiveState;

// ==================== FPSObjectiveSystem Tests ====================

static void testFPSObjCreateAndActivate() {
    std::cout << "\n=== FPS Obj Create And Activate ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    bool created = sys.createObjective("obj1", "station_alpha", "room_3",
                                       "player1", FPSObjType::EliminateHostiles,
                                       "Clear hostiles", 120.0f);
    assertTrue(created, "createObjective returns true");

    bool duplicate = sys.createObjective("obj1", "station_alpha", "room_3",
                                         "player1", FPSObjType::EliminateHostiles);
    assertTrue(!duplicate, "createObjective returns false for duplicate");

    assertTrue(sys.getObjectiveState("obj1") == static_cast<int>(FPSObjState::Inactive),
               "New objective is Inactive");
    assertTrue(approxEqual(sys.getProgress("obj1"), 0.0f), "Initial progress is 0.0");
    assertTrue(!sys.isComplete("obj1"), "New objective is not complete");
    assertTrue(!sys.isFailed("obj1"), "New objective is not failed");

    bool activated = sys.activateObjective("obj1");
    assertTrue(activated, "activateObjective returns true");
    assertTrue(sys.getObjectiveState("obj1") == static_cast<int>(FPSObjState::Active),
               "Objective is Active after activation");

    auto playerObjs = sys.getPlayerObjectives("player1");
    assertTrue(playerObjs.size() == 1, "Player has 1 objective");
    assertTrue(playerObjs[0] == "obj1", "Player objective is obj1");
}

static void testFPSObjEliminateHostiles() {
    std::cout << "\n=== FPS Obj Eliminate Hostiles ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_elim", "ship_beta", "cargo_bay",
                        "player1", FPSObjType::EliminateHostiles, "Kill all hostiles");
    sys.activateObjective("obj_elim");
    sys.setHostileCount("obj_elim", 3);

    // Kill 1 of 3
    bool killed = sys.reportHostileKill("obj_elim");
    assertTrue(killed, "reportHostileKill returns true");
    assertTrue(approxEqual(sys.getProgress("obj_elim"), 1.0f / 3.0f), "Progress is 1/3 after first kill");
    assertTrue(!sys.isComplete("obj_elim"), "Not complete after 1 kill");

    // Kill 2 of 3
    sys.reportHostileKill("obj_elim");
    assertTrue(approxEqual(sys.getProgress("obj_elim"), 2.0f / 3.0f), "Progress is 2/3 after second kill");

    // Kill 3 of 3 → auto-complete
    sys.reportHostileKill("obj_elim");
    assertTrue(approxEqual(sys.getProgress("obj_elim"), 1.0f), "Progress is 1.0 after all kills");
    assertTrue(sys.isComplete("obj_elim"), "Objective auto-completed after all hostiles killed");
    assertTrue(sys.getObjectiveState("obj_elim") == static_cast<int>(FPSObjState::Completed),
               "State is Completed");
}

static void testFPSObjDefendPoint() {
    std::cout << "\n=== FPS Obj Defend Point ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_defend", "station_gamma", "control_room",
                        "player1", FPSObjType::DefendPoint, "Hold the point");
    sys.activateObjective("obj_defend");
    sys.setDefendDuration("obj_defend", 60.0f);

    // Update 20s — 1/3 progress
    sys.update(20.0f);
    assertTrue(approxEqual(sys.getProgress("obj_defend"), 20.0f / 60.0f),
               "Progress is ~0.33 after 20s");
    assertTrue(!sys.isComplete("obj_defend"), "Not complete at 20s");

    // Update another 20s — 2/3 progress
    sys.update(20.0f);
    assertTrue(approxEqual(sys.getProgress("obj_defend"), 40.0f / 60.0f),
               "Progress is ~0.67 after 40s");

    // Update remaining 20s — complete
    sys.update(20.0f);
    assertTrue(approxEqual(sys.getProgress("obj_defend"), 1.0f), "Progress is 1.0 after 60s");
    assertTrue(sys.isComplete("obj_defend"), "Objective completed after defend duration");
}

static void testFPSObjRetrieveItem() {
    std::cout << "\n=== FPS Obj Retrieve Item ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_retrieve", "wreck_delta", "storage",
                        "player1", FPSObjType::RetrieveItem, "Get the keycard");
    sys.activateObjective("obj_retrieve");
    sys.setTargetItem("obj_retrieve", "keycard_42");

    // Wrong item
    bool collected = sys.reportItemCollected("obj_retrieve", "wrong_item");
    assertTrue(!collected, "reportItemCollected fails with wrong item");
    assertTrue(!sys.isComplete("obj_retrieve"), "Not complete with wrong item");
    assertTrue(approxEqual(sys.getProgress("obj_retrieve"), 0.0f), "Progress unchanged with wrong item");

    // Correct item
    collected = sys.reportItemCollected("obj_retrieve", "keycard_42");
    assertTrue(collected, "reportItemCollected succeeds with correct item");
    assertTrue(sys.isComplete("obj_retrieve"), "Objective completed with correct item");
    assertTrue(approxEqual(sys.getProgress("obj_retrieve"), 1.0f), "Progress is 1.0 after item collected");
}

static void testFPSObjTimeLimitFail() {
    std::cout << "\n=== FPS Obj Time Limit Fail ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_timed", "facility_epsilon", "lab",
                        "player1", FPSObjType::Sabotage, "Sabotage the reactor", 30.0f);
    sys.activateObjective("obj_timed");

    // Update within time limit
    sys.update(15.0f);
    assertTrue(!sys.isFailed("obj_timed"), "Not failed at 15s of 30s limit");
    assertTrue(sys.getObjectiveState("obj_timed") == static_cast<int>(FPSObjState::Active),
               "Still Active within time limit");

    // Update past time limit
    sys.update(20.0f);
    assertTrue(sys.isFailed("obj_timed"), "Failed after exceeding time limit");
    assertTrue(sys.getObjectiveState("obj_timed") == static_cast<int>(FPSObjState::Failed),
               "State is Failed");

    // Manual fail on active objective
    sys.createObjective("obj_manual_fail", "facility_epsilon", "lab",
                        "player1", FPSObjType::Escape, "Escape now");
    sys.activateObjective("obj_manual_fail");
    bool failed = sys.failObjective("obj_manual_fail");
    assertTrue(failed, "failObjective returns true on Active objective");
    assertTrue(sys.isFailed("obj_manual_fail"), "Objective marked as Failed");
}

static void testFPSObjTypeNames() {
    std::cout << "\n=== FPS Obj Type Names ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    assertTrue(sys.objectiveTypeName(0) == "EliminateHostiles", "Type 0 = EliminateHostiles");
    assertTrue(sys.objectiveTypeName(1) == "RescueVIP", "Type 1 = RescueVIP");
    assertTrue(sys.objectiveTypeName(2) == "Sabotage", "Type 2 = Sabotage");
    assertTrue(sys.objectiveTypeName(3) == "DefendPoint", "Type 3 = DefendPoint");
    assertTrue(sys.objectiveTypeName(4) == "RetrieveItem", "Type 4 = RetrieveItem");
    assertTrue(sys.objectiveTypeName(5) == "RepairSystem", "Type 5 = RepairSystem");
    assertTrue(sys.objectiveTypeName(6) == "Escape", "Type 6 = Escape");
    assertTrue(sys.objectiveTypeName(99) == "Unknown", "Invalid type = Unknown");

    assertTrue(sys.stateName(0) == "Inactive", "State 0 = Inactive");
    assertTrue(sys.stateName(1) == "Active", "State 1 = Active");
    assertTrue(sys.stateName(2) == "Completed", "State 2 = Completed");
    assertTrue(sys.stateName(3) == "Failed", "State 3 = Failed");
    assertTrue(sys.stateName(-1) == "Unknown", "Invalid state = Unknown");
}

static void testFPSObjMissingEntity() {
    std::cout << "\n=== FPS Obj Missing Entity ===" << std::endl;

    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    assertTrue(!sys.activateObjective("ghost"), "activateObjective fails for nonexistent");
    assertTrue(sys.getObjectiveState("ghost") == -1, "getObjectiveState returns -1 for nonexistent");
    assertTrue(approxEqual(sys.getProgress("ghost"), 0.0f), "getProgress returns 0 for nonexistent");
    assertTrue(!sys.isComplete("ghost"), "isComplete returns false for nonexistent");
    assertTrue(!sys.isFailed("ghost"), "isFailed returns false for nonexistent");
    assertTrue(!sys.reportHostileKill("ghost"), "reportHostileKill fails for nonexistent");
    assertTrue(!sys.reportItemCollected("ghost", "item"), "reportItemCollected fails for nonexistent");
    assertTrue(!sys.reportSabotageComplete("ghost"), "reportSabotageComplete fails for nonexistent");
    assertTrue(!sys.reportExtraction("ghost"), "reportExtraction fails for nonexistent");
    assertTrue(!sys.reportVIPRescued("ghost"), "reportVIPRescued fails for nonexistent");
    assertTrue(!sys.reportRepairComplete("ghost"), "reportRepairComplete fails for nonexistent");
    assertTrue(!sys.failObjective("ghost"), "failObjective fails for nonexistent");
}

void run_fps_objective_system_tests() {
    testFPSObjCreateAndActivate();
    testFPSObjEliminateHostiles();
    testFPSObjDefendPoint();
    testFPSObjRetrieveItem();
    testFPSObjTimeLimitFail();
    testFPSObjTypeNames();
    testFPSObjMissingEntity();
}
