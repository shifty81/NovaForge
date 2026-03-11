// Tests for: PlayerSessionStatsSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/player_session_stats_system.h"

using namespace atlas;

// ==================== PlayerSessionStatsSystem Tests ====================

static void testPlayerSessionStatsInit() {
    std::cout << "\n=== PlayerSessionStats: Init ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(sys.getKills("p1") == 0, "Zero kills initially");
    assertTrue(sys.getLosses("p1") == 0, "Zero losses initially");
    assertTrue(approxEqual(sys.getDamageDealt("p1"), 0.0f), "Zero damage dealt initially");
    assertTrue(approxEqual(sys.getDamageReceived("p1"), 0.0f), "Zero damage received initially");
    assertTrue(sys.getAssists("p1") == 0, "Zero assists initially");
    assertTrue(approxEqual(sys.getIskEarned("p1"), 0.0f), "Zero ISK earned initially");
    assertTrue(approxEqual(sys.getIskSpent("p1"), 0.0f), "Zero ISK spent initially");
    assertTrue(sys.getTradesCompleted("p1") == 0, "Zero trades initially");
    assertTrue(sys.getItemsLooted("p1") == 0, "Zero items looted initially");
    assertTrue(approxEqual(sys.getDistanceTraveled("p1"), 0.0f), "Zero distance initially");
    assertTrue(sys.getJumpsMade("p1") == 0, "Zero jumps initially");
    assertTrue(sys.getWarpsMade("p1") == 0, "Zero warps initially");
    assertTrue(sys.getTotalSessions("p1") == 0, "Zero sessions initially");
    assertTrue(sys.isActive("p1"), "Active by default");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testPlayerSessionStatsStartSession() {
    std::cout << "\n=== PlayerSessionStats: StartSession ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    // Add some stats
    sys.recordKill("p1");
    sys.recordKill("p1");
    sys.recordDamageDealt("p1", 1000.0f);

    assertTrue(sys.startSession("p1"), "Start session succeeds");
    assertTrue(sys.getKills("p1") == 0, "Kills reset on start");
    assertTrue(approxEqual(sys.getDamageDealt("p1"), 0.0f), "Damage reset on start");
    assertTrue(sys.getTotalSessions("p1") == 1, "Total sessions incremented");
    assertTrue(sys.isActive("p1"), "Active after start");

    assertTrue(sys.startSession("p1"), "Second session start succeeds");
    assertTrue(sys.getTotalSessions("p1") == 2, "Total sessions incremented again");
}

static void testPlayerSessionStatsEndSession() {
    std::cout << "\n=== PlayerSessionStats: EndSession ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.recordKill("p1");
    sys.recordDamageDealt("p1", 500.0f);

    assertTrue(sys.endSession("p1"), "End session succeeds");
    assertTrue(!sys.isActive("p1"), "Inactive after end");
    // Stats preserved after end
    assertTrue(sys.getKills("p1") == 1, "Kill count preserved after end");
    assertTrue(approxEqual(sys.getDamageDealt("p1"), 500.0f), "Damage preserved after end");
}

static void testPlayerSessionStatsResetSession() {
    std::cout << "\n=== PlayerSessionStats: ResetSession ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.startSession("p1");
    sys.recordKill("p1");
    sys.recordIskEarned("p1", 1000000.0f);
    sys.endSession("p1");

    assertTrue(sys.resetSession("p1"), "Reset session succeeds");
    assertTrue(sys.isActive("p1"), "Active after reset");
    assertTrue(sys.getKills("p1") == 0, "Kills cleared by reset");
    assertTrue(approxEqual(sys.getIskEarned("p1"), 0.0f), "ISK cleared by reset");
    // total_sessions not reset
    assertTrue(sys.getTotalSessions("p1") == 1, "Total sessions not reset");
}

static void testPlayerSessionStatsCombat() {
    std::cout << "\n=== PlayerSessionStats: Combat ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.recordKill("p1"), "Record kill");
    assertTrue(sys.recordKill("p1"), "Record second kill");
    assertTrue(sys.recordLoss("p1"), "Record loss");
    assertTrue(sys.recordDamageDealt("p1", 5000.0f), "Record damage dealt");
    assertTrue(sys.recordDamageReceived("p1", 3000.0f), "Record damage received");
    assertTrue(sys.recordAssist("p1"), "Record assist");
    assertTrue(sys.recordAssist("p1"), "Record second assist");

    assertTrue(sys.getKills("p1") == 2, "2 kills recorded");
    assertTrue(sys.getLosses("p1") == 1, "1 loss recorded");
    assertTrue(approxEqual(sys.getDamageDealt("p1"), 5000.0f), "5000 damage dealt");
    assertTrue(approxEqual(sys.getDamageReceived("p1"), 3000.0f), "3000 damage received");
    assertTrue(sys.getAssists("p1") == 2, "2 assists recorded");
    assertTrue(approxEqual(sys.getKDRatio("p1"), 2.0f), "KD ratio is 2.0");
}

static void testPlayerSessionStatsKDRatio() {
    std::cout << "\n=== PlayerSessionStats: KDRatio ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    // No kills, no losses → KD = 0
    assertTrue(approxEqual(sys.getKDRatio("p1"), 0.0f), "KD is 0 with no activity");

    // 3 kills, 0 losses → KD = 3.0
    sys.recordKill("p1");
    sys.recordKill("p1");
    sys.recordKill("p1");
    assertTrue(approxEqual(sys.getKDRatio("p1"), 3.0f), "KD is kills when no losses");

    // 3 kills, 1 loss → KD = 3.0
    sys.recordLoss("p1");
    assertTrue(approxEqual(sys.getKDRatio("p1"), 3.0f), "KD ratio 3/1 = 3.0");
}

static void testPlayerSessionStatsEconomy() {
    std::cout << "\n=== PlayerSessionStats: Economy ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.recordIskEarned("p1", 5000000.0f), "Record ISK earned");
    assertTrue(sys.recordIskSpent("p1", 2000000.0f), "Record ISK spent");
    assertTrue(sys.recordTradeCompleted("p1"), "Record trade");
    assertTrue(sys.recordItemLooted("p1", 10), "Record 10 items looted");
    assertTrue(sys.recordItemLooted("p1", 5), "Record 5 more items");

    assertTrue(approxEqual(sys.getIskEarned("p1"), 5000000.0f), "ISK earned correct");
    assertTrue(approxEqual(sys.getIskSpent("p1"), 2000000.0f), "ISK spent correct");
    assertTrue(approxEqual(sys.getNetISK("p1"), 3000000.0f), "Net ISK correct");
    assertTrue(sys.getTradesCompleted("p1") == 1, "1 trade completed");
    assertTrue(sys.getItemsLooted("p1") == 15, "15 items looted total");
}

static void testPlayerSessionStatsTravel() {
    std::cout << "\n=== PlayerSessionStats: Travel ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.recordDistanceTraveled("p1", 10.5f), "Record distance");
    assertTrue(sys.recordDistanceTraveled("p1", 5.0f), "Record more distance");
    assertTrue(sys.recordJump("p1"), "Record jump");
    assertTrue(sys.recordJump("p1"), "Record second jump");
    assertTrue(sys.recordJump("p1"), "Record third jump");
    assertTrue(sys.recordWarp("p1"), "Record warp");
    assertTrue(sys.recordWarp("p1"), "Record second warp");

    assertTrue(approxEqual(sys.getDistanceTraveled("p1"), 15.5f), "15.5 AU traveled");
    assertTrue(sys.getJumpsMade("p1") == 3, "3 jumps made");
    assertTrue(sys.getWarpsMade("p1") == 2, "2 warps made");
}

static void testPlayerSessionStatsElapsedTime() {
    std::cout << "\n=== PlayerSessionStats: ElapsedTime ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    ecs::Entity* entity = world.getEntity("p1");
    (void)entity;
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getElapsedTime("p1"), 10.0f), "Elapsed 10s");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getElapsedTime("p1"), 15.0f), "Elapsed 15s");

    sys.endSession("p1");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getElapsedTime("p1"), 15.0f), "Timer paused after endSession");
}

static void testPlayerSessionStatsInvalidInputs() {
    std::cout << "\n=== PlayerSessionStats: InvalidInputs ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(!sys.recordDamageDealt("p1", -1.0f), "Negative damage dealt rejected");
    assertTrue(!sys.recordDamageReceived("p1", -1.0f), "Negative damage received rejected");
    assertTrue(!sys.recordIskEarned("p1", -1.0f), "Negative ISK earned rejected");
    assertTrue(!sys.recordIskSpent("p1", -1.0f), "Negative ISK spent rejected");
    assertTrue(!sys.recordDistanceTraveled("p1", -1.0f), "Negative distance rejected");
    assertTrue(!sys.recordItemLooted("p1", 0), "Zero item count rejected");
    assertTrue(!sys.recordItemLooted("p1", -1), "Negative item count rejected");
}

static void testPlayerSessionStatsMissing() {
    std::cout << "\n=== PlayerSessionStats: Missing ===" << std::endl;
    ecs::World world;
    systems::PlayerSessionStatsSystem sys(&world);

    assertTrue(!sys.startSession("none"), "StartSession fails on missing");
    assertTrue(!sys.endSession("none"), "EndSession fails on missing");
    assertTrue(!sys.resetSession("none"), "ResetSession fails on missing");
    assertTrue(!sys.recordKill("none"), "RecordKill fails on missing");
    assertTrue(!sys.recordLoss("none"), "RecordLoss fails on missing");
    assertTrue(!sys.recordDamageDealt("none", 1.0f), "RecordDamage fails on missing");
    assertTrue(!sys.recordIskEarned("none", 1.0f), "RecordISK fails on missing");
    assertTrue(!sys.recordJump("none"), "RecordJump fails on missing");
    assertTrue(sys.getKills("none") == 0, "0 kills on missing");
    assertTrue(approxEqual(sys.getDamageDealt("none"), 0.0f), "0 damage on missing");
    assertTrue(approxEqual(sys.getKDRatio("none"), 0.0f), "0 KD on missing");
    assertTrue(sys.getTotalSessions("none") == 0, "0 sessions on missing");
    assertTrue(!sys.isActive("none"), "Not active on missing");
}

void run_player_session_stats_system_tests() {
    testPlayerSessionStatsInit();
    testPlayerSessionStatsStartSession();
    testPlayerSessionStatsEndSession();
    testPlayerSessionStatsResetSession();
    testPlayerSessionStatsCombat();
    testPlayerSessionStatsKDRatio();
    testPlayerSessionStatsEconomy();
    testPlayerSessionStatsTravel();
    testPlayerSessionStatsElapsedTime();
    testPlayerSessionStatsInvalidInputs();
    testPlayerSessionStatsMissing();
}
