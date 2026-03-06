// Tests for: Encounter Spawner System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/encounter_spawner_system.h"

using namespace atlas;

// ==================== Encounter Spawner System Tests ====================

static void testEncounterCreate() {
    std::cout << "\n=== Encounter: Create ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    assertTrue(sys.initialize("enc1", "sol_001"), "Init succeeds");
    assertTrue(sys.getEncounterCount("enc1") == 0, "No encounters");
    assertTrue(sys.getActiveEncounterCount("enc1") == 0, "No active");
    assertTrue(sys.getCompletedEncounterCount("enc1") == 0, "No completed");
    assertTrue(sys.getFailedEncounterCount("enc1") == 0, "No failed");
    assertTrue(approxEqual(sys.getTotalRewardsEarned("enc1"), 0.0), "No rewards");
}

static void testEncounterAddAndQuery() {
    std::cout << "\n=== Encounter: AddAndQuery ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    assertTrue(sys.addEncounter("enc1", "e1", "PirateAmbush", 3, 60.0f), "Add encounter 1");
    assertTrue(sys.addEncounter("enc1", "e2", "TradeEscort", 2, 120.0f), "Add encounter 2");
    assertTrue(sys.getEncounterCount("enc1") == 2, "2 encounters");
    assertTrue(sys.hasEncounter("enc1", "e1"), "Has encounter 1");
    assertTrue(sys.hasEncounter("enc1", "e2"), "Has encounter 2");
    assertTrue(!sys.hasEncounter("enc1", "e3"), "No encounter 3");
    assertTrue(!sys.addEncounter("enc1", "e1", "Dup", 1, 30.0f), "Duplicate rejected");
    assertTrue(sys.getEncounterStatus("enc1", "e1") == 0, "Status is Pending (0)");
}

static void testEncounterMax() {
    std::cout << "\n=== Encounter: Max ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    auto* entity = world.getEntity("enc1");
    auto* state = entity->getComponent<components::EncounterState>();
    state->max_encounters = 2;
    sys.addEncounter("enc1", "e1", "PirateAmbush", 1, 30.0f);
    sys.addEncounter("enc1", "e2", "TradeEscort", 2, 60.0f);
    assertTrue(!sys.addEncounter("enc1", "e3", "CombatChallenge", 3, 90.0f), "Max encounters enforced");
}

static void testEncounterLifecycleComplete() {
    std::cout << "\n=== Encounter: LifecycleComplete ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    sys.addEncounter("enc1", "e1", "PirateAmbush", 3, 60.0f);
    sys.setEncounterReward("enc1", "e1", 5000.0, 3);
    // Activate
    assertTrue(sys.activateEncounter("enc1", "e1", 10.0f), "Activate");
    assertTrue(sys.getEncounterStatus("enc1", "e1") == 1, "Status is Active (1)");
    assertTrue(sys.getActiveEncounterCount("enc1") == 1, "1 active");
    assertTrue(!sys.activateEncounter("enc1", "e1", 15.0f), "Double activate fails");
    // Complete
    assertTrue(sys.completeEncounter("enc1", "e1", 50.0f), "Complete");
    assertTrue(sys.getEncounterStatus("enc1", "e1") == 2, "Status is Completed (2)");
    assertTrue(sys.getActiveEncounterCount("enc1") == 0, "0 active");
    assertTrue(sys.getCompletedEncounterCount("enc1") == 1, "1 completed");
    assertTrue(approxEqual(sys.getTotalRewardsEarned("enc1"), 5000.0), "5000 ISK earned");
    assertTrue(!sys.completeEncounter("enc1", "e1", 55.0f), "Double complete fails");
}

static void testEncounterLifecycleFail() {
    std::cout << "\n=== Encounter: LifecycleFail ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    sys.addEncounter("enc1", "e1", "CombatChallenge", 4, 90.0f);
    sys.activateEncounter("enc1", "e1", 10.0f);
    // Fail
    assertTrue(sys.failEncounter("enc1", "e1", 40.0f), "Fail");
    assertTrue(sys.getEncounterStatus("enc1", "e1") == 3, "Status is Failed (3)");
    assertTrue(sys.getActiveEncounterCount("enc1") == 0, "0 active");
    assertTrue(sys.getFailedEncounterCount("enc1") == 1, "1 failed");
    assertTrue(!sys.failEncounter("enc1", "e1", 45.0f), "Double fail fails");
}

static void testEncounterFailPending() {
    std::cout << "\n=== Encounter: FailPending ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    sys.addEncounter("enc1", "e1", "PirateAmbush", 2, 60.0f);
    assertTrue(!sys.failEncounter("enc1", "e1", 10.0f), "Cannot fail pending encounter");
    assertTrue(!sys.completeEncounter("enc1", "e1", 10.0f), "Cannot complete pending encounter");
}

static void testEncounterRewards() {
    std::cout << "\n=== Encounter: Rewards ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    sys.addEncounter("enc1", "e1", "PirateAmbush", 3, 60.0f);
    sys.addEncounter("enc1", "e2", "TradeEscort", 2, 120.0f);
    assertTrue(sys.setEncounterReward("enc1", "e1", 3000.0, 2), "Set reward e1");
    assertTrue(sys.setEncounterReward("enc1", "e2", 1500.0, 1), "Set reward e2");
    assertTrue(!sys.setEncounterReward("enc1", "e3", 100.0, 0), "Set reward nonexistent fails");
    // Complete both
    sys.activateEncounter("enc1", "e1", 10.0f);
    sys.completeEncounter("enc1", "e1", 50.0f);
    sys.activateEncounter("enc1", "e2", 60.0f);
    sys.completeEncounter("enc1", "e2", 100.0f);
    assertTrue(approxEqual(sys.getTotalRewardsEarned("enc1"), 4500.0), "4500 ISK total");
}

static void testEncounterExpiry() {
    std::cout << "\n=== Encounter: Expiry ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    sys.addEncounter("enc1", "e1", "PirateAmbush", 2, 30.0f); // 30s duration
    sys.activateEncounter("enc1", "e1", 0.0f); // started_at = 0
    assertTrue(sys.getActiveEncounterCount("enc1") == 1, "1 active before expiry");
    // Tick past duration
    sys.update(31.0f);
    assertTrue(sys.getEncounterStatus("enc1", "e1") == 4, "Status is Expired (4)");
    assertTrue(sys.getActiveEncounterCount("enc1") == 0, "0 active after expiry");
}

static void testEncounterUpdate() {
    std::cout << "\n=== Encounter: Update ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    world.createEntity("enc1");
    sys.initialize("enc1", "sol_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("enc1");
    auto* state = entity->getComponent<components::EncounterState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testEncounterMissing() {
    std::cout << "\n=== Encounter: Missing ===" << std::endl;
    ecs::World world;
    systems::EncounterSpawnerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails on missing");
    assertTrue(!sys.addEncounter("nonexistent", "e", "t", 1, 30.0f), "addEncounter fails");
    assertTrue(sys.getEncounterCount("nonexistent") == 0, "0 encounters");
    assertTrue(!sys.hasEncounter("nonexistent", "e"), "hasEncounter false");
    assertTrue(!sys.activateEncounter("nonexistent", "e", 0), "activateEncounter fails");
    assertTrue(!sys.completeEncounter("nonexistent", "e", 0), "completeEncounter fails");
    assertTrue(!sys.failEncounter("nonexistent", "e", 0), "failEncounter fails");
    assertTrue(sys.getEncounterStatus("nonexistent", "e") == -1, "-1 status");
    assertTrue(sys.getActiveEncounterCount("nonexistent") == 0, "0 active");
    assertTrue(sys.getCompletedEncounterCount("nonexistent") == 0, "0 completed");
    assertTrue(sys.getFailedEncounterCount("nonexistent") == 0, "0 failed");
    assertTrue(!sys.setEncounterReward("nonexistent", "e", 0, 0), "setEncounterReward fails");
    assertTrue(approxEqual(sys.getTotalRewardsEarned("nonexistent"), 0.0), "0 rewards");
}

void run_encounter_spawner_system_tests() {
    testEncounterCreate();
    testEncounterAddAndQuery();
    testEncounterMax();
    testEncounterLifecycleComplete();
    testEncounterLifecycleFail();
    testEncounterFailPending();
    testEncounterRewards();
    testEncounterExpiry();
    testEncounterUpdate();
    testEncounterMissing();
}
