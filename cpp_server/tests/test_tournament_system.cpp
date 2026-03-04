// Tests for: TournamentSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/tournament_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== TournamentSystem Tests ====================

static void testTournamentCreate() {
    std::cout << "\n=== Tournament Create ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    bool created = tourneySys.createTournament("tourney_1", "pvp_tourney_1", "Arena Championship", 8, 10000.0, 300.0f);
    assertTrue(created, "Tournament created");
    assertTrue(tourneySys.getStatus("tourney_1") == "registration", "Status is registration");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 0, "Zero participants initially");
}

static void testTournamentRegister() {
    std::cout << "\n=== Tournament Register ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test Tournament", 4, 5000.0, 300.0f);

    assertTrue(tourneySys.registerPlayer("tourney_1", "player_1", "Alice"), "Player 1 registered");
    assertTrue(tourneySys.registerPlayer("tourney_1", "player_2", "Bob"), "Player 2 registered");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 2, "Two participants registered");
    assertTrue(approxEqual(static_cast<float>(tourneySys.getPrizePool("tourney_1")), 10000.0f), "Prize pool is 10K");
}

static void testTournamentMaxParticipants() {
    std::cout << "\n=== Tournament Max Participants ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Small Tourney", 2, 1000.0, 300.0f);

    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    bool third = tourneySys.registerPlayer("tourney_1", "p3", "Charlie");
    assertTrue(!third, "Third player rejected (tournament full)");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 2, "Still 2 participants");
}

static void testTournamentDuplicateRegister() {
    std::cout << "\n=== Tournament Duplicate Register ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);

    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    bool dup = tourneySys.registerPlayer("tourney_1", "p1", "Alice Again");
    assertTrue(!dup, "Duplicate registration rejected");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 1, "Still 1 participant");
}

static void testTournamentStart() {
    std::cout << "\n=== Tournament Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");

    bool started = tourneySys.startTournament("tourney_1");
    assertTrue(started, "Tournament started");
    assertTrue(tourneySys.getStatus("tourney_1") == "active", "Status is active");
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 1, "Round 1 started");
}

static void testTournamentEmptyCannotStart() {
    std::cout << "\n=== Tournament Empty Cannot Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Empty", 8, 0.0, 300.0f);

    bool started = tourneySys.startTournament("tourney_1");
    assertTrue(!started, "Empty tournament cannot start");
    assertTrue(tourneySys.getStatus("tourney_1") == "registration", "Status stays registration");
}

static void testTournamentScoring() {
    std::cout << "\n=== Tournament Scoring ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    tourneySys.recordKill("tourney_1", "p1", 5);
    tourneySys.recordKill("tourney_1", "p2", 3);
    tourneySys.recordKill("tourney_1", "p1", 2);

    assertTrue(tourneySys.getPlayerScore("tourney_1", "p1") == 7, "Player 1 score is 7");
    assertTrue(tourneySys.getPlayerScore("tourney_1", "p2") == 3, "Player 2 score is 3");
}

static void testTournamentElimination() {
    std::cout << "\n=== Tournament Elimination ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.registerPlayer("tourney_1", "p3", "Charlie");
    tourneySys.startTournament("tourney_1");

    assertTrue(tourneySys.getActiveParticipantCount("tourney_1") == 3, "3 active before elimination");
    tourneySys.eliminatePlayer("tourney_1", "p2");
    assertTrue(tourneySys.getActiveParticipantCount("tourney_1") == 2, "2 active after elimination");

    // Eliminated player cannot score
    bool scored = tourneySys.recordKill("tourney_1", "p2", 1);
    assertTrue(!scored, "Eliminated player cannot score");
}

static void testTournamentRoundAdvance() {
    std::cout << "\n=== Tournament Round Advance ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 100.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    tourneySys.recordKill("tourney_1", "p1", 5);
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 1, "Still round 1 before update");

    // Advance past round 1
    tourneySys.update(101.0f);
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 2, "Advanced to round 2");
}

static void testTournamentCompletion() {
    std::cout << "\n=== Tournament Completion ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 1000.0, 50.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    // Advance through all 3 rounds
    tourneySys.update(51.0f);  // end round 1 → start round 2
    tourneySys.update(51.0f);  // end round 2 → start round 3
    tourneySys.update(51.0f);  // end round 3 → completed

    assertTrue(tourneySys.getStatus("tourney_1") == "completed", "Tournament completed after 3 rounds");
}

static void testTournamentRegisterAfterStart() {
    std::cout << "\n=== Tournament Register After Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.startTournament("tourney_1");

    bool late = tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    assertTrue(!late, "Cannot register after tournament starts");
}


void run_tournament_system_tests() {
    testTournamentCreate();
    testTournamentRegister();
    testTournamentMaxParticipants();
    testTournamentDuplicateRegister();
    testTournamentStart();
    testTournamentEmptyCannotStart();
    testTournamentScoring();
    testTournamentElimination();
    testTournamentRoundAdvance();
    testTournamentCompletion();
    testTournamentRegisterAfterStart();
}
