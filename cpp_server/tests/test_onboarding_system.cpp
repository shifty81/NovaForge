// Tests for: Onboarding System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/onboarding_system.h"

using namespace atlas;

// ==================== Onboarding System Tests ====================

static void testOnboardingCreate() {
    std::cout << "\n=== Onboarding: Create ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    assertTrue(sys.initialize("ob1", "player_001"), "Init succeeds");
    assertTrue(sys.getCurrentPhase("ob1") == 0, "Phase is NotStarted");
    assertTrue(sys.getObjectiveCount("ob1") == 0, "No objectives");
    assertTrue(sys.getCompletedObjectiveCount("ob1") == 0, "No completed objectives");
    assertTrue(sys.getHintCount("ob1") == 0, "No hints");
    assertTrue(sys.getShownHintCount("ob1") == 0, "No shown hints");
    assertTrue(!sys.isTutorialComplete("ob1"), "Not complete");
    assertTrue(!sys.isTutorialSkipped("ob1"), "Not skipped");
    assertTrue(approxEqual(sys.getCompletionTime("ob1"), 0.0f), "No completion time");
}

static void testOnboardingStartTutorial() {
    std::cout << "\n=== Onboarding: StartTutorial ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    assertTrue(sys.startTutorial("ob1", 10.0f), "Start tutorial");
    assertTrue(sys.getCurrentPhase("ob1") == 1, "Phase is Welcome");
    assertTrue(!sys.startTutorial("ob1", 20.0f), "Double start fails");
}

static void testOnboardingAdvancePhase() {
    std::cout << "\n=== Onboarding: AdvancePhase ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    sys.startTutorial("ob1", 0.0f);
    assertTrue(sys.getCurrentPhase("ob1") == 1, "Welcome");
    assertTrue(sys.advancePhase("ob1"), "Advance to Undocking");
    assertTrue(sys.getCurrentPhase("ob1") == 2, "Undocking");
    assertTrue(sys.advancePhase("ob1"), "Advance to BasicFlight");
    assertTrue(sys.getCurrentPhase("ob1") == 3, "BasicFlight");
    assertTrue(sys.advancePhase("ob1"), "Advance to Mining");
    assertTrue(sys.getCurrentPhase("ob1") == 4, "Mining");
    assertTrue(sys.advancePhase("ob1"), "Advance to Trading");
    assertTrue(sys.getCurrentPhase("ob1") == 5, "Trading");
    assertTrue(sys.advancePhase("ob1"), "Advance to ShipFitting");
    assertTrue(sys.getCurrentPhase("ob1") == 6, "ShipFitting");
    assertTrue(sys.advancePhase("ob1"), "Advance to Combat");
    assertTrue(sys.getCurrentPhase("ob1") == 7, "Combat");
    assertTrue(sys.advancePhase("ob1"), "Advance to Warping");
    assertTrue(sys.getCurrentPhase("ob1") == 8, "Warping");
    assertTrue(sys.advancePhase("ob1"), "Advance to Completed");
    assertTrue(sys.getCurrentPhase("ob1") == 9, "Completed");
    assertTrue(sys.isTutorialComplete("ob1"), "Tutorial is complete");
    assertTrue(!sys.advancePhase("ob1"), "Advance past completed fails");
}

static void testOnboardingSkipTutorial() {
    std::cout << "\n=== Onboarding: SkipTutorial ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    sys.startTutorial("ob1", 0.0f);
    sys.advancePhase("ob1");  // to Undocking
    assertTrue(sys.skipTutorial("ob1"), "Skip succeeds");
    assertTrue(sys.isTutorialSkipped("ob1"), "Is skipped");
    assertTrue(sys.getCurrentPhase("ob1") == 9, "Phase is Completed");
    assertTrue(!sys.skipTutorial("ob1"), "Double skip fails");
    assertTrue(!sys.advancePhase("ob1"), "Advance after skip fails");
}

static void testOnboardingSkipAlreadyComplete() {
    std::cout << "\n=== Onboarding: SkipAlreadyComplete ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    sys.startTutorial("ob1", 0.0f);
    // Advance all the way through
    for (int i = 0; i < 8; i++) sys.advancePhase("ob1");
    assertTrue(sys.isTutorialComplete("ob1"), "Complete");
    assertTrue(!sys.skipTutorial("ob1"), "Skip already complete fails");
}

static void testOnboardingObjectives() {
    std::cout << "\n=== Onboarding: Objectives ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    assertTrue(sys.addObjective("ob1", "obj1", "Undock from station", 2), "Add obj 1");
    assertTrue(sys.addObjective("ob1", "obj2", "Fly to asteroid belt", 3), "Add obj 2");
    assertTrue(sys.addObjective("ob1", "obj3", "Mine an asteroid", 4), "Add obj 3");
    assertTrue(sys.getObjectiveCount("ob1") == 3, "3 objectives");
    assertTrue(!sys.addObjective("ob1", "obj1", "Dup", 2), "Duplicate rejected");
    assertTrue(!sys.isObjectiveComplete("ob1", "obj1"), "obj1 not complete");
    assertTrue(sys.completeObjective("ob1", "obj1", 15.0f), "Complete obj1");
    assertTrue(sys.isObjectiveComplete("ob1", "obj1"), "obj1 complete");
    assertTrue(sys.getCompletedObjectiveCount("ob1") == 1, "1 completed");
    assertTrue(!sys.completeObjective("ob1", "obj1", 20.0f), "Double complete fails");
    assertTrue(!sys.completeObjective("ob1", "nonexistent", 25.0f), "Complete nonexistent fails");
}

static void testOnboardingObjectiveMax() {
    std::cout << "\n=== Onboarding: ObjectiveMax ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    auto* entity = world.getEntity("ob1");
    auto* state = entity->getComponent<components::OnboardingState>();
    state->max_objectives = 2;
    sys.addObjective("ob1", "o1", "A", 1);
    sys.addObjective("ob1", "o2", "B", 2);
    assertTrue(!sys.addObjective("ob1", "o3", "C", 3), "Max objectives enforced");
}

static void testOnboardingHints() {
    std::cout << "\n=== Onboarding: Hints ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    assertTrue(sys.addHint("ob1", "hint1", "Press F to undock"), "Add hint 1");
    assertTrue(sys.addHint("ob1", "hint2", "Use WASD to fly"), "Add hint 2");
    assertTrue(sys.getHintCount("ob1") == 2, "2 hints");
    assertTrue(!sys.addHint("ob1", "hint1", "Dup"), "Duplicate rejected");
    assertTrue(!sys.isHintShown("ob1", "hint1"), "hint1 not shown");
    assertTrue(sys.showHint("ob1", "hint1", 5.0f), "Show hint1");
    assertTrue(sys.isHintShown("ob1", "hint1"), "hint1 shown");
    assertTrue(sys.getShownHintCount("ob1") == 1, "1 shown");
    assertTrue(!sys.showHint("ob1", "hint1", 6.0f), "Double show fails");
    assertTrue(!sys.showHint("ob1", "nonexistent", 7.0f), "Show nonexistent fails");
}

static void testOnboardingHintMax() {
    std::cout << "\n=== Onboarding: HintMax ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    auto* entity = world.getEntity("ob1");
    auto* state = entity->getComponent<components::OnboardingState>();
    state->max_hints = 1;
    sys.addHint("ob1", "h1", "A");
    assertTrue(!sys.addHint("ob1", "h2", "B"), "Max hints enforced");
}

static void testOnboardingUpdate() {
    std::cout << "\n=== Onboarding: Update ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("ob1");
    auto* state = entity->getComponent<components::OnboardingState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testOnboardingCompletionTime() {
    std::cout << "\n=== Onboarding: CompletionTime ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    world.createEntity("ob1");
    sys.initialize("ob1", "player_001");
    sys.startTutorial("ob1", 0.0f);
    sys.update(10.0f);  // simulate 10 seconds of play
    for (int i = 0; i < 8; i++) sys.advancePhase("ob1");
    assertTrue(sys.isTutorialComplete("ob1"), "Complete");
    assertTrue(approxEqual(sys.getCompletionTime("ob1"), 10.0f), "Completion time is 10s");
}

static void testOnboardingMissing() {
    std::cout << "\n=== Onboarding: Missing ===" << std::endl;
    ecs::World world;
    systems::OnboardingSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails on missing");
    assertTrue(!sys.startTutorial("nonexistent", 0), "startTutorial fails");
    assertTrue(!sys.advancePhase("nonexistent"), "advancePhase fails");
    assertTrue(!sys.skipTutorial("nonexistent"), "skipTutorial fails");
    assertTrue(sys.getCurrentPhase("nonexistent") == -1, "-1 phase");
    assertTrue(!sys.addObjective("nonexistent", "o", "d", 1), "addObjective fails");
    assertTrue(!sys.completeObjective("nonexistent", "o", 0), "completeObjective fails");
    assertTrue(!sys.isObjectiveComplete("nonexistent", "o"), "isObjectiveComplete false");
    assertTrue(sys.getObjectiveCount("nonexistent") == 0, "0 objectives");
    assertTrue(sys.getCompletedObjectiveCount("nonexistent") == 0, "0 completed");
    assertTrue(!sys.addHint("nonexistent", "h", "t"), "addHint fails");
    assertTrue(!sys.showHint("nonexistent", "h", 0), "showHint fails");
    assertTrue(!sys.isHintShown("nonexistent", "h"), "isHintShown false");
    assertTrue(sys.getHintCount("nonexistent") == 0, "0 hints");
    assertTrue(sys.getShownHintCount("nonexistent") == 0, "0 shown");
    assertTrue(!sys.isTutorialComplete("nonexistent"), "not complete");
    assertTrue(!sys.isTutorialSkipped("nonexistent"), "not skipped");
    assertTrue(approxEqual(sys.getCompletionTime("nonexistent"), 0.0f), "0 completion time");
}

void run_onboarding_system_tests() {
    testOnboardingCreate();
    testOnboardingStartTutorial();
    testOnboardingAdvancePhase();
    testOnboardingSkipTutorial();
    testOnboardingSkipAlreadyComplete();
    testOnboardingObjectives();
    testOnboardingObjectiveMax();
    testOnboardingHints();
    testOnboardingHintMax();
    testOnboardingUpdate();
    testOnboardingCompletionTime();
    testOnboardingMissing();
}
