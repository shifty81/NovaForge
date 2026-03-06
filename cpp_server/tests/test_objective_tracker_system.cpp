// Tests for: Objective Tracker System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/objective_tracker_system.h"

using namespace atlas;

// ==================== Objective Tracker System Tests ====================

static void testObjectiveTrackerCreate() {
    std::cout << "\n=== ObjectiveTracker: Create ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getObjectiveCount("p1") == 0, "No objectives");
    assertTrue(sys.getCompletedCount("p1") == 0, "No completed");
    assertTrue(approxEqual(sys.getOverallCompletion("p1"), 0.0f), "0% completion");
    assertTrue(sys.getActiveObjectiveId("p1").empty(), "No active objective");
}

static void testObjectiveTrackerAddAndQuery() {
    std::cout << "\n=== ObjectiveTracker: AddAndQuery ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.addObjective("p1", "obj1", "Mine 100 Veldspar", "Mining", 100.0f, 200.0f), "Add obj1");
    assertTrue(sys.addObjective("p1", "obj2", "Dock at Station Alpha", "Mission", 500.0f, 300.0f), "Add obj2");
    assertTrue(sys.getObjectiveCount("p1") == 2, "2 objectives");
    assertTrue(sys.hasObjective("p1", "obj1"), "Has obj1");
    assertTrue(sys.hasObjective("p1", "obj2"), "Has obj2");
    assertTrue(!sys.hasObjective("p1", "obj3"), "No obj3");
    assertTrue(!sys.addObjective("p1", "obj1", "Dup", "Dup", 0, 0), "Duplicate rejected");
}

static void testObjectiveTrackerAutoActive() {
    std::cout << "\n=== ObjectiveTracker: AutoActive ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addObjective("p1", "obj1", "First objective", "Mission", 0, 0);
    assertTrue(sys.getActiveObjectiveId("p1") == "obj1", "First obj auto-active");
    sys.addObjective("p1", "obj2", "Second objective", "Mission", 0, 0);
    assertTrue(sys.getActiveObjectiveId("p1") == "obj1", "Still obj1 active");
    assertTrue(sys.setActiveObjective("p1", "obj2"), "Switch active");
    assertTrue(sys.getActiveObjectiveId("p1") == "obj2", "Now obj2 active");
    assertTrue(!sys.setActiveObjective("p1", "nonexistent"), "Can't set nonexistent active");
}

static void testObjectiveTrackerMax() {
    std::cout << "\n=== ObjectiveTracker: Max ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::ObjectiveTrackerState>();
    state->max_objectives = 2;
    sys.addObjective("p1", "obj1", "First", "Mission", 0, 0);
    sys.addObjective("p1", "obj2", "Second", "Mission", 0, 0);
    assertTrue(!sys.addObjective("p1", "obj3", "Third", "Mission", 0, 0), "Max enforced");
}

static void testObjectiveTrackerProgress() {
    std::cout << "\n=== ObjectiveTracker: Progress ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addObjective("p1", "obj1", "Mine ore", "Mining", 0, 0);
    assertTrue(approxEqual(sys.getProgress("p1", "obj1"), 0.0f), "Initial progress 0");
    assertTrue(sys.updateProgress("p1", "obj1", 0.5f), "Set 50%");
    assertTrue(approxEqual(sys.getProgress("p1", "obj1"), 0.5f), "50% progress");
    assertTrue(sys.updateProgress("p1", "obj1", 1.5f), "Clamp to 1.0");
    assertTrue(approxEqual(sys.getProgress("p1", "obj1"), 1.0f), "Clamped at 1.0");
    assertTrue(sys.updateProgress("p1", "obj1", -0.5f), "Clamp to 0.0");
    assertTrue(approxEqual(sys.getProgress("p1", "obj1"), 0.0f), "Clamped at 0.0");
    assertTrue(!sys.updateProgress("p1", "nonexistent", 0.5f), "Nonexistent fails");
}

static void testObjectiveTrackerComplete() {
    std::cout << "\n=== ObjectiveTracker: Complete ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addObjective("p1", "obj1", "Mine ore", "Mining", 0, 0);
    sys.updateProgress("p1", "obj1", 0.5f);
    assertTrue(sys.completeObjective("p1", "obj1", 30.0f), "Complete obj1");
    assertTrue(sys.isObjectiveComplete("p1", "obj1"), "obj1 complete");
    assertTrue(approxEqual(sys.getProgress("p1", "obj1"), 1.0f), "Progress set to 1.0");
    assertTrue(sys.getCompletedCount("p1") == 1, "1 completed");
    assertTrue(!sys.completeObjective("p1", "obj1", 35.0f), "Double complete fails");
    assertTrue(!sys.updateProgress("p1", "obj1", 0.3f), "Can't update completed");
}

static void testObjectiveTrackerRemove() {
    std::cout << "\n=== ObjectiveTracker: Remove ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addObjective("p1", "obj1", "First", "Mission", 0, 0);
    sys.addObjective("p1", "obj2", "Second", "Mission", 0, 0);
    assertTrue(sys.getActiveObjectiveId("p1") == "obj1", "obj1 active");
    assertTrue(sys.removeObjective("p1", "obj1"), "Remove obj1");
    assertTrue(sys.getObjectiveCount("p1") == 1, "1 left");
    assertTrue(!sys.hasObjective("p1", "obj1"), "obj1 gone");
    assertTrue(sys.getActiveObjectiveId("p1").empty(), "Active cleared");
    assertTrue(!sys.removeObjective("p1", "obj1"), "Double remove fails");
}

static void testObjectiveTrackerDistance() {
    std::cout << "\n=== ObjectiveTracker: Distance ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addObjective("p1", "obj1", "Go to station", "Mission", 300.0f, 400.0f);
    assertTrue(sys.updatePlayerPosition("p1", 0.0f, 0.0f), "Set player pos");
    float dist = sys.getDistanceToObjective("p1", "obj1");
    assertTrue(approxEqual(dist, 500.0f), "Distance is 500 (3-4-5 triangle)");
    assertTrue(approxEqual(sys.getDistanceToActive("p1"), 500.0f), "Active distance is 500");
    assertTrue(sys.getDistanceToObjective("p1", "nonexistent") < 0.0f, "Nonexistent returns -1");
}

static void testObjectiveTrackerOverallCompletion() {
    std::cout << "\n=== ObjectiveTracker: OverallCompletion ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addObjective("p1", "obj1", "Task A", "Mission", 0, 0);
    sys.addObjective("p1", "obj2", "Task B", "Mission", 0, 0);
    sys.updateProgress("p1", "obj1", 0.5f);
    sys.updateProgress("p1", "obj2", 1.0f);
    assertTrue(approxEqual(sys.getOverallCompletion("p1"), 0.75f), "75% overall");
}

static void testObjectiveTrackerUpdate() {
    std::cout << "\n=== ObjectiveTracker: Update ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::ObjectiveTrackerState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testObjectiveTrackerMissing() {
    std::cout << "\n=== ObjectiveTracker: Missing ===" << std::endl;
    ecs::World world;
    systems::ObjectiveTrackerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.addObjective("nonexistent", "o", "d", "c", 0, 0), "addObjective fails");
    assertTrue(sys.getObjectiveCount("nonexistent") == 0, "0 objectives");
    assertTrue(!sys.hasObjective("nonexistent", "o"), "hasObjective false");
    assertTrue(!sys.removeObjective("nonexistent", "o"), "removeObjective fails");
    assertTrue(!sys.updateProgress("nonexistent", "o", 0.5f), "updateProgress fails");
    assertTrue(approxEqual(sys.getProgress("nonexistent", "o"), 0.0f), "0 progress");
    assertTrue(!sys.completeObjective("nonexistent", "o", 0), "completeObjective fails");
    assertTrue(!sys.isObjectiveComplete("nonexistent", "o"), "isComplete false");
    assertTrue(!sys.setActiveObjective("nonexistent", "o"), "setActive fails");
    assertTrue(sys.getActiveObjectiveId("nonexistent").empty(), "No active");
    assertTrue(!sys.updatePlayerPosition("nonexistent", 0, 0), "updatePos fails");
    assertTrue(sys.getDistanceToObjective("nonexistent", "o") < 0.0f, "-1 distance");
    assertTrue(sys.getDistanceToActive("nonexistent") < 0.0f, "-1 active distance");
    assertTrue(sys.getCompletedCount("nonexistent") == 0, "0 completed");
    assertTrue(approxEqual(sys.getOverallCompletion("nonexistent"), 0.0f), "0 completion");
}

void run_objective_tracker_system_tests() {
    testObjectiveTrackerCreate();
    testObjectiveTrackerAddAndQuery();
    testObjectiveTrackerAutoActive();
    testObjectiveTrackerMax();
    testObjectiveTrackerProgress();
    testObjectiveTrackerComplete();
    testObjectiveTrackerRemove();
    testObjectiveTrackerDistance();
    testObjectiveTrackerOverallCompletion();
    testObjectiveTrackerUpdate();
    testObjectiveTrackerMissing();
}
