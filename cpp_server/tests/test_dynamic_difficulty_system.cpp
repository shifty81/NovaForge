// Tests for: DynamicDifficultySystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/dynamic_difficulty_system.h"

using namespace atlas;

// ==================== DynamicDifficultySystem Tests ====================

static void testDifficultyDefaultState() {
    std::cout << "\n=== DynamicDifficulty: DefaultState ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::DynamicDifficultyState>(e);

    assertTrue(approxEqual(state->player_combat_rating, 1.0f), "Default combat rating 1.0");
    assertTrue(approxEqual(state->ship_power_level, 1.0f), "Default ship power 1.0");
    assertTrue(state->encounters_won == 0, "Zero wins");
    assertTrue(state->encounters_lost == 0, "Zero losses");
    assertTrue(state->consecutive_wins == 0, "Zero consecutive wins");
    assertTrue(state->consecutive_losses == 0, "Zero consecutive losses");
    assertTrue(sys.getTotalEncounters("player1") == 0, "Zero total encounters");
}

static void testDifficultyRecordWin() {
    std::cout << "\n=== DynamicDifficulty: RecordWin ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::DynamicDifficultyState>(e);

    assertTrue(sys.recordWin("player1"), "recordWin succeeds");
    assertTrue(sys.getConsecutiveWins("player1") == 1, "1 consecutive win");
    assertTrue(sys.getConsecutiveLosses("player1") == 0, "0 consecutive losses");
    assertTrue(sys.getTotalEncounters("player1") == 1, "1 total encounter");

    sys.recordWin("player1");
    sys.recordWin("player1");
    assertTrue(sys.getConsecutiveWins("player1") == 3, "3 consecutive wins");
}

static void testDifficultyRecordLoss() {
    std::cout << "\n=== DynamicDifficulty: RecordLoss ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::DynamicDifficultyState>(e);

    assertTrue(sys.recordLoss("player1"), "recordLoss succeeds");
    assertTrue(sys.getConsecutiveLosses("player1") == 1, "1 consecutive loss");
    assertTrue(sys.getConsecutiveWins("player1") == 0, "0 consecutive wins");

    sys.recordLoss("player1");
    assertTrue(sys.getConsecutiveLosses("player1") == 2, "2 consecutive losses");
}

static void testDifficultyStreakReset() {
    std::cout << "\n=== DynamicDifficulty: StreakReset ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::DynamicDifficultyState>(e);

    sys.recordWin("player1");
    sys.recordWin("player1");
    assertTrue(sys.getConsecutiveWins("player1") == 2, "2 wins");

    // Loss resets win streak
    sys.recordLoss("player1");
    assertTrue(sys.getConsecutiveWins("player1") == 0, "Wins reset after loss");
    assertTrue(sys.getConsecutiveLosses("player1") == 1, "1 loss after reset");

    // Win resets loss streak
    sys.recordWin("player1");
    assertTrue(sys.getConsecutiveLosses("player1") == 0, "Losses reset after win");
    assertTrue(sys.getConsecutiveWins("player1") == 1, "1 win after reset");
}

static void testDifficultyCalculateMultiplier() {
    std::cout << "\n=== DynamicDifficulty: CalculateMultiplier ===" << std::endl;

    // Default player: rating=1, power=1, no streaks -> base * sqrt(1*1) = 1.0
    float m1 = systems::DynamicDifficultySystem::calculateMultiplier(
        1.0f, 1.0f, 0, 0, 1.0f, 0.1f, 0.5f, 3.0f);
    assertTrue(approxEqual(m1, 1.0f), "Default multiplier is 1.0");

    // Strong player: rating=4, power=4, no streaks -> 1.0 * sqrt(16) = 4.0 clamped to 3.0
    float m2 = systems::DynamicDifficultySystem::calculateMultiplier(
        4.0f, 4.0f, 0, 0, 1.0f, 0.1f, 0.5f, 3.0f);
    assertTrue(approxEqual(m2, 3.0f), "Strong player clamped to max 3.0");

    // Weak player: rating=0.1, power=0.1, no streaks -> 1.0 * sqrt(0.01) = 0.1 clamped to 0.5
    float m3 = systems::DynamicDifficultySystem::calculateMultiplier(
        0.1f, 0.1f, 0, 0, 1.0f, 0.1f, 0.5f, 3.0f);
    assertTrue(approxEqual(m3, 0.5f), "Weak player clamped to min 0.5");

    // Win streak: 5 wins, adjustment=0.1 -> +0.5
    float m4 = systems::DynamicDifficultySystem::calculateMultiplier(
        1.0f, 1.0f, 5, 0, 1.0f, 0.1f, 0.5f, 3.0f);
    assertTrue(approxEqual(m4, 1.5f), "Win streak increases difficulty");

    // Loss streak: 5 losses, adjustment=0.1 -> -0.5
    float m5 = systems::DynamicDifficultySystem::calculateMultiplier(
        1.0f, 1.0f, 0, 5, 1.0f, 0.1f, 0.5f, 3.0f);
    assertTrue(approxEqual(m5, 0.5f), "Loss streak decreases difficulty (clamped to min)");
}

static void testDifficultyUpdateRecomputes() {
    std::cout << "\n=== DynamicDifficulty: UpdateRecomputes ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::DynamicDifficultyState>(e);

    state->player_combat_rating = 2.0f;
    state->ship_power_level = 2.0f;

    sys.update(1.0f);

    // sqrt(2*2) = 2.0, base * 2 = 2.0
    float expected = 2.0f;
    assertTrue(approxEqual(sys.getEncounterMultiplier("player1"), expected, 0.05f),
               "Update recomputes multiplier");
    assertTrue(approxEqual(state->time_since_last_encounter, 1.0f), "Time tracked");
}

static void testDifficultyWinRecordUpdatesMultiplier() {
    std::cout << "\n=== DynamicDifficulty: WinRecordUpdatesMultiplier ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::DynamicDifficultyState>(e);

    float before = sys.getEncounterMultiplier("player1");
    sys.recordWin("player1");
    float after = sys.getEncounterMultiplier("player1");

    assertTrue(after >= before, "Difficulty increases or stays same after win");
}

static void testDifficultyLossRecordUpdatesMultiplier() {
    std::cout << "\n=== DynamicDifficulty: LossRecordUpdatesMultiplier ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::DynamicDifficultyState>(e);

    float before = sys.getEncounterMultiplier("player1");
    sys.recordLoss("player1");
    float after = sys.getEncounterMultiplier("player1");

    assertTrue(after <= before, "Difficulty decreases or stays same after loss");
}

static void testDifficultyClamping() {
    std::cout << "\n=== DynamicDifficulty: Clamping ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::DynamicDifficultyState>(e);

    // Push difficulty very high
    state->player_combat_rating = 10.0f;
    state->ship_power_level = 10.0f;
    state->consecutive_wins = 100;
    sys.update(0.016f);
    assertTrue(state->encounter_multiplier <= state->max_difficulty,
               "Multiplier clamped to max");

    // Push difficulty very low
    state->player_combat_rating = 0.1f;
    state->ship_power_level = 0.1f;
    state->consecutive_wins = 0;
    state->consecutive_losses = 100;
    sys.update(0.016f);
    assertTrue(state->encounter_multiplier >= state->min_difficulty,
               "Multiplier clamped to min");
}

static void testDifficultyMissing() {
    std::cout << "\n=== DynamicDifficulty: Missing ===" << std::endl;
    ecs::World world;
    systems::DynamicDifficultySystem sys(&world);

    assertTrue(!sys.recordWin("missing"), "recordWin on missing");
    assertTrue(!sys.recordLoss("missing"), "recordLoss on missing");
    assertTrue(approxEqual(sys.computeDifficulty("missing"), 1.0f), "computeDifficulty on missing");
    assertTrue(approxEqual(sys.getEncounterMultiplier("missing"), 1.0f), "getEncounterMultiplier on missing");
    assertTrue(sys.getConsecutiveWins("missing") == 0, "getConsecutiveWins on missing");
    assertTrue(sys.getConsecutiveLosses("missing") == 0, "getConsecutiveLosses on missing");
    assertTrue(sys.getTotalEncounters("missing") == 0, "getTotalEncounters on missing");
}

void run_dynamic_difficulty_system_tests() {
    testDifficultyDefaultState();
    testDifficultyRecordWin();
    testDifficultyRecordLoss();
    testDifficultyStreakReset();
    testDifficultyCalculateMultiplier();
    testDifficultyUpdateRecomputes();
    testDifficultyWinRecordUpdatesMultiplier();
    testDifficultyLossRecordUpdatesMultiplier();
    testDifficultyClamping();
    testDifficultyMissing();
}
