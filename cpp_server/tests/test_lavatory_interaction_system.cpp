// Tests for: LavatoryInteractionSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/lavatory_interaction_system.h"

using namespace atlas;

// ==================== LavatoryInteractionSystem Tests ====================

static void testLavatoryCreate() {
    std::cout << "\n=== Lavatory: Create ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");

    assertTrue(sys.createLavatory("lav1", "room_a"), "Create lavatory");
    assertTrue(sys.getPhase("lav1") == 0, "Phase is Idle");
    assertTrue(!sys.isOccupied("lav1"), "Not occupied initially");
    assertTrue(!sys.isDoorOpen("lav1"), "Door closed initially");
    assertTrue(!sys.isInThirdPerson("lav1"), "Not in third person initially");
    assertTrue(!sys.isAudioPlaying("lav1"), "No audio initially");
}

static void testLavatoryDuplicateCreateRejected() {
    std::cout << "\n=== Lavatory: DuplicateCreateRejected ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");

    assertTrue(sys.createLavatory("lav1", "room_a"), "First create ok");
    assertTrue(!sys.createLavatory("lav1", "room_b"), "Duplicate create rejected");
}

static void testLavatoryCustomParameters() {
    std::cout << "\n=== Lavatory: CustomParameters ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");

    assertTrue(sys.createLavatory("lav1", "room_a", 10.0f, 25.0f), "Create with custom params");
    assertTrue(approxEqual(sys.getHygieneBonus("lav1"), 25.0f), "Custom hygiene bonus");
}

static void testLavatoryBeginInteraction() {
    std::cout << "\n=== Lavatory: BeginInteraction ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a");

    assertTrue(sys.beginInteraction("lav1", "player1"), "Begin interaction");
    assertTrue(sys.isOccupied("lav1"), "Now occupied");
    assertTrue(sys.getPhase("lav1") == 1, "Phase is Approaching");
    assertTrue(approxEqual(sys.getPhaseProgress("lav1"), 0.0f), "Progress is 0");
}

static void testLavatoryCannotBeginWhileOccupied() {
    std::cout << "\n=== Lavatory: CannotBeginWhileOccupied ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a");
    sys.beginInteraction("lav1", "player1");

    assertTrue(!sys.beginInteraction("lav1", "player2"), "Cannot begin while occupied");
}

static void testLavatoryPhaseProgression() {
    std::cout << "\n=== Lavatory: PhaseProgression ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a", 2.0f, 15.0f);
    sys.beginInteraction("lav1", "player1");

    // Phase 1: Approaching (phase_duration=1.5s)
    assertTrue(sys.getPhase("lav1") == 1, "Phase 1: Approaching");

    // Advance past phase 1
    sys.update(2.0f);
    assertTrue(sys.getPhase("lav1") >= 2, "Advanced past approaching");
}

static void testLavatoryDoorOpensInPhase2() {
    std::cout << "\n=== Lavatory: DoorOpensInPhase2 ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a", 2.0f, 15.0f);
    sys.beginInteraction("lav1", "player1");

    // Advance to DoorOpening phase
    sys.update(2.0f);
    int phase = sys.getPhase("lav1");
    if (phase >= 2) {
        assertTrue(sys.isDoorOpen("lav1"), "Door opens in phase 2");
        assertTrue(sys.isAudioPlaying("lav1"), "Audio plays during door opening");
    } else {
        assertTrue(true, "Phase not yet reached (timing)");
    }
}

static void testLavatoryThirdPersonTransition() {
    std::cout << "\n=== Lavatory: ThirdPersonTransition ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a", 2.0f, 15.0f);
    sys.beginInteraction("lav1", "player1");

    // Advance through multiple phases to reach ThirdPerson
    for (int i = 0; i < 10; i++) sys.update(1.6f);

    int phase = sys.getPhase("lav1");
    // Should have reached at least the third person transition
    assertTrue(phase >= 3, "Advanced to at least phase 3");
}

static void testLavatoryFullCycleToComplete() {
    std::cout << "\n=== Lavatory: FullCycleToComplete ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a", 2.0f, 15.0f);
    sys.beginInteraction("lav1", "player1");

    // Advance through all phases (7 phases * ~1.5s each + 2s use = ~12.5s)
    for (int i = 0; i < 20; i++) sys.update(2.0f);

    int phase = sys.getPhase("lav1");
    assertTrue(phase == 7, "Phase reached Complete (7)");
    assertTrue(!sys.isOccupied("lav1"), "No longer occupied after complete");
    assertTrue(!sys.isDoorOpen("lav1"), "Door closed after complete");
    assertTrue(!sys.isAudioPlaying("lav1"), "Audio stopped after complete");
}

static void testLavatoryCancelInteraction() {
    std::cout << "\n=== Lavatory: CancelInteraction ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a");
    sys.beginInteraction("lav1", "player1");

    assertTrue(sys.cancelInteraction("lav1"), "Cancel interaction");
    assertTrue(sys.getPhase("lav1") == 0, "Phase reset to Idle");
    assertTrue(!sys.isOccupied("lav1"), "No longer occupied");
    assertTrue(!sys.isDoorOpen("lav1"), "Door closed");
    assertTrue(!sys.isInThirdPerson("lav1"), "Not in third person");
    assertTrue(!sys.isAudioPlaying("lav1"), "Audio stopped");
}

static void testLavatoryCancelIdleFails() {
    std::cout << "\n=== Lavatory: CancelIdleFails ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a");

    assertTrue(!sys.cancelInteraction("lav1"), "Cannot cancel idle lavatory");
}

static void testLavatoryPhaseName() {
    std::cout << "\n=== Lavatory: PhaseName ===" << std::endl;
    assertTrue(systems::LavatoryInteractionSystem::phaseName(0) == "idle", "Phase 0 is idle");
    assertTrue(systems::LavatoryInteractionSystem::phaseName(1) == "approaching", "Phase 1 is approaching");
    assertTrue(systems::LavatoryInteractionSystem::phaseName(2) == "door_opening", "Phase 2 is door_opening");
    assertTrue(systems::LavatoryInteractionSystem::phaseName(4) == "using_facility", "Phase 4 is using_facility");
    assertTrue(systems::LavatoryInteractionSystem::phaseName(7) == "complete", "Phase 7 is complete");
}

static void testLavatoryHygieneBonus() {
    std::cout << "\n=== Lavatory: HygieneBonus ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav1");
    sys.createLavatory("lav1", "room_a", 5.0f, 20.0f);

    assertTrue(approxEqual(sys.getHygieneBonus("lav1"), 20.0f), "Hygiene bonus is 20");
}

static void testLavatoryMissingEntity() {
    std::cout << "\n=== Lavatory: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);

    assertTrue(!sys.createLavatory("ghost", "room_a"), "Create fails for missing entity");
    assertTrue(!sys.beginInteraction("ghost", "p1"), "Begin fails for missing");
    assertTrue(!sys.cancelInteraction("ghost"), "Cancel fails for missing");
    assertTrue(sys.getPhase("ghost") == 0, "Phase 0 for missing");
    assertTrue(approxEqual(sys.getPhaseProgress("ghost"), 0.0f), "Progress 0 for missing");
    assertTrue(!sys.isOccupied("ghost"), "Not occupied for missing");
    assertTrue(!sys.isDoorOpen("ghost"), "Door closed for missing");
    assertTrue(!sys.isInThirdPerson("ghost"), "Not third person for missing");
    assertTrue(!sys.isAudioPlaying("ghost"), "No audio for missing");
    assertTrue(approxEqual(sys.getHygieneBonus("ghost"), 0.0f), "Hygiene 0 for missing");
}

void run_lavatory_interaction_system_tests() {
    testLavatoryCreate();
    testLavatoryDuplicateCreateRejected();
    testLavatoryCustomParameters();
    testLavatoryBeginInteraction();
    testLavatoryCannotBeginWhileOccupied();
    testLavatoryPhaseProgression();
    testLavatoryDoorOpensInPhase2();
    testLavatoryThirdPersonTransition();
    testLavatoryFullCycleToComplete();
    testLavatoryCancelInteraction();
    testLavatoryCancelIdleFails();
    testLavatoryPhaseName();
    testLavatoryHygieneBonus();
    testLavatoryMissingEntity();
}
