// Tests for: LavatoryInteraction Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/lavatory_interaction_system.h"

using namespace atlas;

// ==================== LavatoryInteraction Tests ====================

static void testLavatoryInit() {
    std::cout << "\n=== LavatoryInteraction: Init ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    assertTrue(sys.createLavatory("lav_1", "room_bathroom", 5.0f, 15.0f), "Lavatory created");
    assertTrue(sys.getPhase("lav_1") == 0, "Phase is idle");
    assertTrue(!sys.isOccupied("lav_1"), "Not occupied");
    assertTrue(!sys.createLavatory("lav_1", "room_2"), "Duplicate creation fails");
}

static void testLavatoryBeginInteraction() {
    std::cout << "\n=== LavatoryInteraction: Begin ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom");
    assertTrue(sys.beginInteraction("lav_1", "player_1"), "Interaction started");
    assertTrue(sys.isOccupied("lav_1"), "Now occupied");
    assertTrue(sys.getPhase("lav_1") == 1, "Phase is approaching");
    assertTrue(!sys.beginInteraction("lav_1", "player_2"), "Can't use occupied lavatory");
}

static void testLavatoryDoorTransition() {
    std::cout << "\n=== LavatoryInteraction: Door Transition ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 5.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Advance through Approaching phase (phase_duration = 1.5s)
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.isDoorOpen("lav_1"), "Door opened during sequence");
}

static void testLavatoryThirdPerson() {
    std::cout << "\n=== LavatoryInteraction: Third Person ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 5.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Advance through Approaching + DoorOpening + into TransitionToThirdPerson
    for (int i = 0; i < 6; i++) sys.update(1.0f);
    // Should reach or pass through TransitionToThirdPerson
    assertTrue(sys.getPhase("lav_1") >= 3, "Advanced past door opening");
}

static void testLavatoryFullCycle() {
    std::cout << "\n=== LavatoryInteraction: Full Cycle ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 2.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Run enough ticks to complete the full cycle
    // 7 phases × 1.5s each (except UsingFacility which is 2.0s) ≈ 12.5s
    for (int i = 0; i < 30; i++) sys.update(1.0f);
    assertTrue(sys.getPhase("lav_1") == 7, "Reached complete phase");
    assertTrue(!sys.isOccupied("lav_1"), "No longer occupied after completion");
}

static void testLavatoryCancel() {
    std::cout << "\n=== LavatoryInteraction: Cancel ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom");
    sys.beginInteraction("lav_1", "player_1");
    sys.update(1.0f);
    assertTrue(sys.cancelInteraction("lav_1"), "Interaction cancelled");
    assertTrue(!sys.isOccupied("lav_1"), "Not occupied after cancel");
    assertTrue(sys.getPhase("lav_1") == 0, "Phase reset to idle");
    assertTrue(!sys.cancelInteraction("lav_1"), "Can't cancel idle");
}

static void testLavatoryAudio() {
    std::cout << "\n=== LavatoryInteraction: Audio ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 2.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Initially no audio
    assertTrue(!sys.isAudioPlaying("lav_1"), "No audio initially");
    // Advance past approaching into door opening
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.isAudioPlaying("lav_1"), "Audio playing during sequence");
}

static void testLavatoryHygiene() {
    std::cout << "\n=== LavatoryInteraction: Hygiene Bonus ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 5.0f, 20.0f);
    assertTrue(approxEqual(sys.getHygieneBonus("lav_1"), 20.0f), "Custom hygiene bonus");
    world.createEntity("lav_2");
    sys.createLavatory("lav_2", "room_2");
    assertTrue(approxEqual(sys.getHygieneBonus("lav_2"), 15.0f), "Default hygiene bonus");
}

static void testLavatoryPhaseNames() {
    std::cout << "\n=== LavatoryInteraction: Phase Names ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    assertTrue(sys.phaseName(0) == "idle", "Phase 0 name");
    assertTrue(sys.phaseName(1) == "approaching", "Phase 1 name");
    assertTrue(sys.phaseName(4) == "using_facility", "Phase 4 name");
    assertTrue(sys.phaseName(7) == "complete", "Phase 7 name");
}

static void testLavatoryMissing() {
    std::cout << "\n=== LavatoryInteraction: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    assertTrue(!sys.createLavatory("nonexistent", "room"), "Create fails on missing");
    assertTrue(!sys.beginInteraction("nonexistent", "p"), "Begin fails on missing");
    assertTrue(sys.getPhase("nonexistent") == 0, "Phase 0 on missing");
    assertTrue(!sys.isOccupied("nonexistent"), "Not occupied on missing");
}


void run_lavatory_interaction_tests() {
    testLavatoryInit();
    testLavatoryBeginInteraction();
    testLavatoryDoorTransition();
    testLavatoryThirdPerson();
    testLavatoryFullCycle();
    testLavatoryCancel();
    testLavatoryAudio();
    testLavatoryHygiene();
    testLavatoryPhaseNames();
    testLavatoryMissing();
}
