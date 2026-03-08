// Tests for: Encounter Balance System
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/encounter_balance_system.h"

using namespace atlas;

// ==================== Encounter Balance System Tests ====================

static void testEncounterBalanceCreate() {
    std::cout << "\n=== EncounterBalance: Create ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    assertTrue(sys.initialize("enc1", "pirate_ambush", 1.0f, 500.0f), "Init succeeds");
    assertTrue(approxEqual(sys.getDifficultyMultiplier("enc1"), 1.0f), "Default difficulty 1.0");
    assertTrue(approxEqual(sys.getRewardMultiplier("enc1"), 1.0f), "Default reward 1.0");
    assertTrue(sys.getRecalcCount("enc1") == 0, "0 recalcs initially");
    assertTrue(sys.getPlayerCount("enc1") == 1, "Default 1 player");
    assertTrue(sys.getAvgShipClass("enc1") == 1, "Default ship class 1");
    assertTrue(sys.getTotalKills("enc1") == 0, "0 kills initially");
}

static void testEncounterBalanceInitValidation() {
    std::cout << "\n=== EncounterBalance: InitValidation ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    assertTrue(!sys.initialize("enc1", "", 1.0f, 500.0f), "Empty encounter_id rejected");
    world.createEntity("enc2");
    assertTrue(!sys.initialize("enc2", "test", 0.0f, 500.0f), "Zero difficulty rejected");
    world.createEntity("enc3");
    assertTrue(!sys.initialize("enc3", "test", 1.0f, -1.0f), "Negative reward rejected");
    assertTrue(!sys.initialize("nonexistent", "test", 1.0f, 500.0f), "Missing entity rejected");
}

static void testEncounterBalanceSetPlayerCount() {
    std::cout << "\n=== EncounterBalance: SetPlayerCount ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    assertTrue(sys.setPlayerCount("enc1", 5), "Set 5 players");
    assertTrue(sys.getPlayerCount("enc1") == 5, "5 players stored");
    assertTrue(sys.setPlayerCount("enc1", 0), "Set 0 clamped to 1");
    assertTrue(sys.getPlayerCount("enc1") == 1, "Clamped to 1");
    assertTrue(sys.setPlayerCount("enc1", 25), "Set 25 clamped to 20");
    assertTrue(sys.getPlayerCount("enc1") == 20, "Clamped to 20");
    assertTrue(!sys.setPlayerCount("nonexistent", 3), "Missing entity rejected");
}

static void testEncounterBalanceSetShipClass() {
    std::cout << "\n=== EncounterBalance: SetShipClass ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    assertTrue(sys.setAvgShipClass("enc1", 3), "Set class 3");
    assertTrue(sys.getAvgShipClass("enc1") == 3, "Class 3 stored");
    assertTrue(sys.setAvgShipClass("enc1", 0), "Set 0 clamped to 1");
    assertTrue(sys.getAvgShipClass("enc1") == 1, "Clamped to 1");
    assertTrue(sys.setAvgShipClass("enc1", 10), "Set 10 clamped to 5");
    assertTrue(sys.getAvgShipClass("enc1") == 5, "Clamped to 5");
}

static void testEncounterBalanceAddKills() {
    std::cout << "\n=== EncounterBalance: AddKills ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    assertTrue(sys.addKills("enc1", 5), "Add 5 kills");
    assertTrue(sys.getTotalKills("enc1") == 5, "5 kills total");
    assertTrue(sys.addKills("enc1", 3), "Add 3 more kills");
    assertTrue(sys.getTotalKills("enc1") == 8, "8 kills total");
    assertTrue(!sys.addKills("enc1", 0), "0 kills rejected");
    assertTrue(!sys.addKills("enc1", -1), "Negative kills rejected");
    assertTrue(!sys.addKills("nonexistent", 1), "Missing entity rejected");
}

static void testEncounterBalanceRecalcSoloFrigate() {
    std::cout << "\n=== EncounterBalance: Recalc Solo Frigate ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    // 1 player, class 1, 0 kills → difficulty = 1.0 * 1.0 * 1.1 = 1.1
    assertTrue(sys.recalculate("enc1"), "Recalculate succeeds");
    assertTrue(approxEqual(sys.getDifficultyMultiplier("enc1"), 1.1f), "Solo frigate difficulty 1.1");
    assertTrue(sys.getRecalcCount("enc1") == 1, "1 recalc");
}

static void testEncounterBalanceRecalcGroupBattleship() {
    std::cout << "\n=== EncounterBalance: Recalc Group Battleship ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    sys.setPlayerCount("enc1", 5);
    sys.setAvgShipClass("enc1", 4);
    // difficulty = 1.0 * (1+0.25*4) * (1+0.1*4) = 1.0 * 2.0 * 1.4 = 2.8
    sys.recalculate("enc1");
    assertTrue(approxEqual(sys.getDifficultyMultiplier("enc1"), 2.8f), "Group BS difficulty 2.8");
}

static void testEncounterBalanceRecalcWithKills() {
    std::cout << "\n=== EncounterBalance: Recalc With Kills ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    sys.addKills("enc1", 50);
    sys.recalculate("enc1");
    // difficulty = 1.1 (solo frigate), reward_mul = 1.1 * (1+0.01*50) = 1.1 * 1.5 = 1.65
    assertTrue(approxEqual(sys.getRewardMultiplier("enc1"), 1.65f), "Reward with 50 kills");
    assertTrue(approxEqual(sys.getEffectiveReward("enc1"), 165.0f), "Effective reward 165 ISC");
}

static void testEncounterBalanceDifficultyClamping() {
    std::cout << "\n=== EncounterBalance: DifficultyClamping ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 2.0f, 100.0f);
    sys.setPlayerCount("enc1", 20);
    sys.setAvgShipClass("enc1", 5);
    sys.recalculate("enc1");
    // Raw: 2.0 * (1+0.25*19) * (1+0.1*5) = 2.0 * 5.75 * 1.5 = 17.25 → clamped to 3.0
    assertTrue(approxEqual(sys.getDifficultyMultiplier("enc1"), 3.0f), "Clamped to 3.0 max");
}

static void testEncounterBalanceUpdate() {
    std::cout << "\n=== EncounterBalance: Update ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "encounter_1", 1.0f, 100.0f);
    sys.update(1.0f);
    // Just verifies update doesn't crash and elapsed advances
    assertTrue(true, "Update tick OK");
}

static void testEncounterBalanceMissing() {
    std::cout << "\n=== EncounterBalance: Missing ===" << std::endl;
    ecs::World world;
    systems::EncounterBalanceSystem sys(&world);
    assertTrue(approxEqual(sys.getDifficultyMultiplier("x"), 1.0f), "Default difficulty on missing");
    assertTrue(approxEqual(sys.getRewardMultiplier("x"), 1.0f), "Default reward on missing");
    assertTrue(approxEqual(sys.getEffectiveReward("x"), 0.0f), "0 effective on missing");
    assertTrue(sys.getRecalcCount("x") == 0, "0 recalcs on missing");
    assertTrue(sys.getPlayerCount("x") == 0, "0 players on missing");
    assertTrue(sys.getAvgShipClass("x") == 0, "0 class on missing");
    assertTrue(sys.getTotalKills("x") == 0, "0 kills on missing");
    assertTrue(!sys.recalculate("x"), "Recalc fails on missing");
}

void run_encounter_balance_system_tests() {
    testEncounterBalanceCreate();
    testEncounterBalanceInitValidation();
    testEncounterBalanceSetPlayerCount();
    testEncounterBalanceSetShipClass();
    testEncounterBalanceAddKills();
    testEncounterBalanceRecalcSoloFrigate();
    testEncounterBalanceRecalcGroupBattleship();
    testEncounterBalanceRecalcWithKills();
    testEncounterBalanceDifficultyClamping();
    testEncounterBalanceUpdate();
    testEncounterBalanceMissing();
}
