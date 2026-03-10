// Tests for: TutorialSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/tutorial_system.h"

using namespace atlas;

// ==================== TutorialSystem Tests ====================

static void testTutorialInit() {
    std::cout << "\n=== Tutorial: Init ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(!sys.isActive("p1"), "Not active initially");
    assertTrue(!sys.isComplete("p1"), "Not complete initially");
    assertTrue(sys.getCurrentStepId("p1").empty(), "No current step initially");
    assertTrue(sys.getCompletedStepCount("p1") == 0, "Zero completed steps");
    assertTrue(sys.getTotalStepCount("p1") == 0, "Zero total steps");
    assertTrue(approxEqual(sys.getElapsed("p1"), 0.0f), "Elapsed starts at 0");
}

static void testTutorialInitFails() {
    std::cout << "\n=== Tutorial: InitFails ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testTutorialAddStep() {
    std::cout << "\n=== Tutorial: AddStep ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.addStep("p1", "move", "Learn to move"), "Add first step");
    assertTrue(sys.addStep("p1", "shoot", "Learn to shoot"), "Add second step");
    assertTrue(sys.getTotalStepCount("p1") == 2, "Two steps stored");
    assertTrue(!sys.addStep("p1", "move", "Duplicate"), "Duplicate step rejected");
    assertTrue(!sys.addStep("p1", "", "Empty ID"), "Empty step ID rejected");
    assertTrue(sys.getTotalStepCount("p1") == 2, "Count unchanged after rejections");
}

static void testTutorialStartTutorial() {
    std::cout << "\n=== Tutorial: StartTutorial ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(!sys.startTutorial("p1"), "Start fails with no steps");
    sys.addStep("p1", "move", "Learn to move");
    assertTrue(sys.startTutorial("p1"), "Start succeeds with steps");
    assertTrue(sys.isActive("p1"), "Tutorial is active");
    assertTrue(sys.getCurrentStepId("p1") == "move", "First step is current");
    assertTrue(!sys.startTutorial("p1"), "Double start rejected");
}

static void testTutorialCompleteStep() {
    std::cout << "\n=== Tutorial: CompleteStep ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addStep("p1", "move", "Learn to move");
    sys.addStep("p1", "shoot", "Learn to shoot");
    sys.startTutorial("p1");

    assertTrue(!sys.completeStep("p1", "shoot"), "Cannot skip to wrong step");
    assertTrue(sys.completeStep("p1", "move"), "Complete first step");
    assertTrue(sys.getCompletedStepCount("p1") == 1, "Completed count is 1");
    assertTrue(sys.getCurrentStepId("p1") == "shoot", "Second step is now current");
    assertTrue(!sys.completeStep("p1", "move"), "Cannot re-complete completed step");
}

static void testTutorialFullCompletion() {
    std::cout << "\n=== Tutorial: FullCompletion ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addStep("p1", "move", "Learn to move");
    sys.addStep("p1", "dock", "Learn to dock");
    sys.startTutorial("p1");

    sys.completeStep("p1", "move");
    assertTrue(sys.isActive("p1"), "Still active after first step");
    assertTrue(!sys.isComplete("p1"), "Not complete after first step");

    sys.completeStep("p1", "dock");
    assertTrue(!sys.isActive("p1"), "Inactive after last step");
    assertTrue(sys.isComplete("p1"), "Complete after last step");
    assertTrue(sys.getCompletedStepCount("p1") == 2, "Both steps counted");
    assertTrue(sys.getCurrentStepId("p1").empty(), "No current step when complete");
}

static void testTutorialSkip() {
    std::cout << "\n=== Tutorial: Skip ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addStep("p1", "move", "Learn to move");
    sys.addStep("p1", "shoot", "Learn to shoot");
    sys.addStep("p1", "dock", "Learn to dock");
    sys.startTutorial("p1");

    assertTrue(!sys.skipTutorial("nonexistent"), "Skip fails on missing entity");
    assertTrue(sys.skipTutorial("p1"), "Skip succeeds while active");
    assertTrue(!sys.isActive("p1"), "Not active after skip");
    assertTrue(sys.isComplete("p1"), "Complete after skip");
    auto* comp = world.getEntity("p1")->getComponent<components::TutorialState>();
    assertTrue(comp->is_skipped, "is_skipped flag set");
    assertTrue(sys.getCompletedStepCount("p1") == 3, "All steps counted as completed");
}

static void testTutorialReset() {
    std::cout << "\n=== Tutorial: Reset ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addStep("p1", "move", "Learn to move");
    sys.startTutorial("p1");
    sys.completeStep("p1", "move");

    assertTrue(sys.resetTutorial("p1"), "Reset succeeds");
    assertTrue(!sys.isActive("p1"), "Not active after reset");
    assertTrue(!sys.isComplete("p1"), "Not complete after reset");
    assertTrue(sys.getCompletedStepCount("p1") == 0, "Completed count reset");
    assertTrue(approxEqual(sys.getElapsed("p1"), 0.0f), "Elapsed reset to 0");
    // Can start again after reset
    assertTrue(sys.startTutorial("p1"), "Can start after reset");
}

static void testTutorialElapsedTimer() {
    std::cout << "\n=== Tutorial: ElapsedTimer ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addStep("p1", "move", "Learn to move");

    sys.update(5.0f);
    assertTrue(approxEqual(sys.getElapsed("p1"), 0.0f), "Elapsed not ticking before start");

    sys.startTutorial("p1");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getElapsed("p1"), 5.0f), "Elapsed ticks while active");

    sys.update(3.0f);
    assertTrue(approxEqual(sys.getElapsed("p1"), 8.0f), "Elapsed accumulates correctly");
}

static void testTutorialAddStepAfterStart() {
    std::cout << "\n=== Tutorial: AddStepAfterStart ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addStep("p1", "move", "Learn to move");
    sys.startTutorial("p1");

    assertTrue(!sys.addStep("p1", "dock", "Learn to dock"),
               "Cannot add steps while tutorial is active");
    assertTrue(sys.getTotalStepCount("p1") == 1, "Step count unchanged");
}

static void testTutorialMissing() {
    std::cout << "\n=== Tutorial: Missing ===" << std::endl;
    ecs::World world;
    systems::TutorialSystem sys(&world);

    assertTrue(!sys.startTutorial("nonexistent"), "Start fails on missing");
    assertTrue(!sys.completeStep("nonexistent", "s1"), "CompleteStep fails on missing");
    assertTrue(!sys.skipTutorial("nonexistent"), "Skip fails on missing");
    assertTrue(!sys.resetTutorial("nonexistent"), "Reset fails on missing");
    assertTrue(!sys.isActive("nonexistent"), "Not active on missing");
    assertTrue(!sys.isComplete("nonexistent"), "Not complete on missing");
    assertTrue(sys.getCurrentStepId("nonexistent").empty(), "Empty step on missing");
    assertTrue(sys.getCompletedStepCount("nonexistent") == 0, "0 completed on missing");
    assertTrue(sys.getTotalStepCount("nonexistent") == 0, "0 total on missing");
    assertTrue(approxEqual(sys.getElapsed("nonexistent"), 0.0f), "0 elapsed on missing");
}

void run_tutorial_system_tests() {
    testTutorialInit();
    testTutorialInitFails();
    testTutorialAddStep();
    testTutorialStartTutorial();
    testTutorialCompleteStep();
    testTutorialFullCompletion();
    testTutorialSkip();
    testTutorialReset();
    testTutorialElapsedTimer();
    testTutorialAddStepAfterStart();
    testTutorialMissing();
}
