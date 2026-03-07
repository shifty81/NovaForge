// Tests for: Session Progression System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/session_progression_system.h"

using namespace atlas;

// ==================== Session Progression System Tests ====================

static void testProgressionCreate() {
    std::cout << "\n=== Progression: Create ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    assertTrue(sys.initialize("sp1", "player_001"), "Init succeeds");
    assertTrue(sys.getMilestoneCount("sp1") == 0, "No milestones");
    assertTrue(sys.getStatisticCount("sp1") == 0, "No statistics");
    assertTrue(sys.getActivityCount("sp1") == 0, "No activities");
    assertTrue(approxEqual(sys.getSessionDuration("sp1"), 0.0f), "No duration");
    assertTrue(!sys.isSessionFinalized("sp1"), "Not finalized");
}

static void testProgressionMilestones() {
    std::cout << "\n=== Progression: Milestones ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    assertTrue(sys.recordMilestone("sp1", "first_undock", "Undocked for the first time", 5.0f), "Record milestone 1");
    assertTrue(sys.recordMilestone("sp1", "first_mine", "Mined first asteroid", 30.0f), "Record milestone 2");
    assertTrue(sys.getMilestoneCount("sp1") == 2, "2 milestones");
    assertTrue(sys.isMilestoneReached("sp1", "first_undock"), "Undock reached");
    assertTrue(sys.isMilestoneReached("sp1", "first_mine"), "Mine reached");
    assertTrue(!sys.isMilestoneReached("sp1", "first_kill"), "Kill not reached");
    assertTrue(!sys.recordMilestone("sp1", "first_undock", "Dup", 10.0f), "Duplicate rejected");
}

static void testProgressionMilestoneMax() {
    std::cout << "\n=== Progression: MilestoneMax ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    auto* entity = world.getEntity("sp1");
    auto* state = entity->getComponent<components::SessionProgression>();
    state->max_milestones = 2;
    sys.recordMilestone("sp1", "m1", "A", 1.0f);
    sys.recordMilestone("sp1", "m2", "B", 2.0f);
    assertTrue(!sys.recordMilestone("sp1", "m3", "C", 3.0f), "Max milestones enforced");
}

static void testProgressionStatistics() {
    std::cout << "\n=== Progression: Statistics ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    assertTrue(sys.addStatistic("sp1", "isc_earned", 1000.0), "Add ISC earned");
    assertTrue(sys.addStatistic("sp1", "asteroids_mined", 5.0), "Add asteroids mined");
    assertTrue(sys.getStatisticCount("sp1") == 2, "2 statistics");
    assertTrue(approxEqual(sys.getStatistic("sp1", "isc_earned"), 1000.0), "ISC = 1000");
    assertTrue(approxEqual(sys.getStatistic("sp1", "asteroids_mined"), 5.0), "Mined = 5");
    // Accumulate
    assertTrue(sys.addStatistic("sp1", "isc_earned", 500.0), "Accumulate ISC");
    assertTrue(approxEqual(sys.getStatistic("sp1", "isc_earned"), 1500.0), "ISC = 1500");
    assertTrue(sys.getStatisticCount("sp1") == 2, "Still 2 statistics");
    // Missing stat
    assertTrue(approxEqual(sys.getStatistic("sp1", "nonexistent"), 0.0), "Missing stat = 0");
}

static void testProgressionActivities() {
    std::cout << "\n=== Progression: Activities ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    assertTrue(sys.logActivity("sp1", "undock", "Undocked from Jita", 5.0f), "Log undock");
    assertTrue(sys.logActivity("sp1", "mine", "Mined Veldspar", 30.0f), "Log mine");
    assertTrue(sys.logActivity("sp1", "trade", "Sold ore for 500 ISC", 60.0f), "Log trade");
    assertTrue(sys.getActivityCount("sp1") == 3, "3 activities");
    assertTrue(sys.getLastActivityType("sp1") == "trade", "Last activity is trade");
}

static void testProgressionActivityRolling() {
    std::cout << "\n=== Progression: ActivityRolling ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    auto* entity = world.getEntity("sp1");
    auto* state = entity->getComponent<components::SessionProgression>();
    state->max_activities = 3;
    sys.logActivity("sp1", "a1", "First", 1.0f);
    sys.logActivity("sp1", "a2", "Second", 2.0f);
    sys.logActivity("sp1", "a3", "Third", 3.0f);
    assertTrue(sys.getActivityCount("sp1") == 3, "3 activities at max");
    sys.logActivity("sp1", "a4", "Fourth", 4.0f);
    assertTrue(sys.getActivityCount("sp1") == 3, "Still 3 after rolling");
    assertTrue(sys.getLastActivityType("sp1") == "a4", "Last is a4");
}

static void testProgressionFinalize() {
    std::cout << "\n=== Progression: Finalize ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    sys.update(10.0f);
    assertTrue(!sys.isSessionFinalized("sp1"), "Not finalized");
    assertTrue(sys.finalizeSession("sp1", 10.0f), "Finalize session");
    assertTrue(sys.isSessionFinalized("sp1"), "Is finalized");
    assertTrue(!sys.finalizeSession("sp1", 20.0f), "Double finalize fails");
}

static void testProgressionUpdate() {
    std::cout << "\n=== Progression: Update ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    world.createEntity("sp1");
    sys.initialize("sp1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    assertTrue(approxEqual(sys.getSessionDuration("sp1"), 3.5f), "Duration 3.5s");
}

static void testProgressionMissing() {
    std::cout << "\n=== Progression: Missing ===" << std::endl;
    ecs::World world;
    systems::SessionProgressionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails on missing");
    assertTrue(!sys.recordMilestone("nonexistent", "m", "d", 0), "recordMilestone fails");
    assertTrue(!sys.isMilestoneReached("nonexistent", "m"), "isMilestoneReached false");
    assertTrue(sys.getMilestoneCount("nonexistent") == 0, "0 milestones");
    assertTrue(!sys.addStatistic("nonexistent", "k", 1.0), "addStatistic fails");
    assertTrue(approxEqual(sys.getStatistic("nonexistent", "k"), 0.0), "getStatistic 0");
    assertTrue(sys.getStatisticCount("nonexistent") == 0, "0 statistics");
    assertTrue(!sys.logActivity("nonexistent", "t", "d", 0), "logActivity fails");
    assertTrue(sys.getActivityCount("nonexistent") == 0, "0 activities");
    assertTrue(sys.getLastActivityType("nonexistent") == "", "empty last activity");
    assertTrue(approxEqual(sys.getSessionDuration("nonexistent"), 0.0f), "0 duration");
    assertTrue(!sys.finalizeSession("nonexistent", 0), "finalizeSession fails");
    assertTrue(!sys.isSessionFinalized("nonexistent"), "not finalized");
}

void run_session_progression_system_tests() {
    testProgressionCreate();
    testProgressionMilestones();
    testProgressionMilestoneMax();
    testProgressionStatistics();
    testProgressionActivities();
    testProgressionActivityRolling();
    testProgressionFinalize();
    testProgressionUpdate();
    testProgressionMissing();
}
