// Tests for: Anomaly Escalation System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/anomaly_escalation_system.h"

using namespace atlas;

// ==================== Anomaly Escalation System Tests ====================

static void testAnomalyEscalationCreate() {
    std::cout << "\n=== AnomalyEscalation: Create ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    assertTrue(sys.initialize("anom1", "sys_jita", "player1"), "Init succeeds");
    assertTrue(sys.getTierCount("anom1") == 0, "No tiers initially");
    assertTrue(sys.getCurrentTier("anom1") == 0, "Tier 0 initially");
    assertTrue(sys.getState("anom1") == "Idle", "State is Idle");
    assertTrue(sys.getTotalSitesCleared("anom1") == 0, "No sites cleared");
    assertTrue(sys.getTotalEscalationsTriggered("anom1") == 0, "No escalations triggered");
    assertTrue(sys.getTotalEscalationsCompleted("anom1") == 0, "No escalations completed");
    assertTrue(sys.getTotalEscalationsFailed("anom1") == 0, "No escalations failed");
}

static void testAnomalyEscalationAddTiers() {
    std::cout << "\n=== AnomalyEscalation: AddTiers ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    assertTrue(sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5), "Add tier 0");
    assertTrue(sys.addTier("anom1", 1, "Combat", 1.5f, 1.5f, 8), "Add tier 1");
    assertTrue(sys.addTier("anom1", 2, "Combat", 2.0f, 2.5f, 12), "Add tier 2");
    assertTrue(sys.getTierCount("anom1") == 3, "3 tiers added");
}

static void testAnomalyEscalationDuplicateTier() {
    std::cout << "\n=== AnomalyEscalation: DuplicateTier ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    assertTrue(!sys.addTier("anom1", 0, "Relic", 1.2f, 1.3f, 6), "Duplicate tier rejected");
    assertTrue(sys.getTierCount("anom1") == 1, "Still 1 tier");
}

static void testAnomalyEscalationStartSite() {
    std::cout << "\n=== AnomalyEscalation: StartSite ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    assertTrue(!sys.startSite("anom1"), "Can't start with no tiers");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    assertTrue(sys.startSite("anom1"), "Start site succeeds");
    assertTrue(sys.getState("anom1") == "SiteActive", "State is SiteActive");
}

static void testAnomalyEscalationClearWithEscalation() {
    std::cout << "\n=== AnomalyEscalation: ClearWithEscalation ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    sys.addTier("anom1", 1, "Combat", 1.5f, 1.5f, 8);

    auto* entity = world.getEntity("anom1");
    auto* comp = entity->getComponent<components::AnomalyEscalation>();
    comp->escalation_chance = 1.0f; // force escalation

    sys.startSite("anom1");
    sys.clearSite("anom1", 0.0f); // roll=0 < chance=1.0, triggers escalation
    assertTrue(sys.getState("anom1") == "Escalating", "State is Escalating");
    assertTrue(sys.getCurrentTier("anom1") == 1, "Advanced to tier 1");
    assertTrue(sys.getTotalSitesCleared("anom1") == 1, "1 site cleared");
    assertTrue(sys.getTotalEscalationsTriggered("anom1") == 1, "1 escalation triggered");
}

static void testAnomalyEscalationClearNoEscalation() {
    std::cout << "\n=== AnomalyEscalation: ClearNoEscalation ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    sys.addTier("anom1", 1, "Combat", 1.5f, 1.5f, 8);

    auto* entity = world.getEntity("anom1");
    auto* comp = entity->getComponent<components::AnomalyEscalation>();
    comp->escalation_chance = 0.0f; // no escalation

    sys.startSite("anom1");
    sys.clearSite("anom1", 0.5f); // roll=0.5 >= chance=0.0, no escalation
    assertTrue(sys.getState("anom1") == "Cleared", "State is Cleared");
    assertTrue(sys.getCurrentTier("anom1") == 0, "Still at tier 0");
    assertTrue(sys.getTotalEscalationsTriggered("anom1") == 0, "No escalation triggered");
}

static void testAnomalyEscalationTimerUpdate() {
    std::cout << "\n=== AnomalyEscalation: TimerUpdate ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    sys.addTier("anom1", 1, "Combat", 2.0f, 2.0f, 10);

    auto* entity = world.getEntity("anom1");
    auto* comp = entity->getComponent<components::AnomalyEscalation>();
    comp->escalation_chance = 1.0f;
    comp->escalation_delay = 5.0f;

    sys.startSite("anom1");
    sys.clearSite("anom1", 0.0f);
    assertTrue(sys.getState("anom1") == "Escalating", "State is Escalating");

    sys.update(3.0f); // 3 of 5 seconds
    assertTrue(sys.getState("anom1") == "Escalating", "Still Escalating after 3s");

    sys.update(3.0f); // now 6s total, past the 5s delay
    assertTrue(sys.getState("anom1") == "EscalationReady", "EscalationReady after delay");
}

static void testAnomalyEscalationDifficultyReward() {
    std::cout << "\n=== AnomalyEscalation: DifficultyReward ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    sys.addTier("anom1", 1, "Combat", 2.0f, 3.0f, 10);

    assertTrue(approxEqual(sys.getDifficultyMultiplier("anom1"), 1.0f), "Tier 0 difficulty 1.0");
    assertTrue(approxEqual(sys.getRewardMultiplier("anom1"), 1.0f), "Tier 0 reward 1.0");

    auto* entity = world.getEntity("anom1");
    auto* comp = entity->getComponent<components::AnomalyEscalation>();
    comp->escalation_chance = 1.0f;
    sys.startSite("anom1");
    sys.clearSite("anom1", 0.0f);

    assertTrue(approxEqual(sys.getDifficultyMultiplier("anom1"), 2.0f), "Tier 1 difficulty 2.0");
    assertTrue(approxEqual(sys.getRewardMultiplier("anom1"), 3.0f), "Tier 1 reward 3.0");
}

static void testAnomalyEscalationCompleteAndFail() {
    std::cout << "\n=== AnomalyEscalation: CompleteAndFail ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    world.createEntity("anom2");
    sys.initialize("anom1", "sys_jita", "player1");
    sys.initialize("anom2", "sys_jita", "player2");
    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    sys.addTier("anom2", 0, "Combat", 1.0f, 1.0f, 5);

    sys.startSite("anom1");
    assertTrue(sys.completeEscalation("anom1"), "Complete escalation succeeds");
    assertTrue(sys.getState("anom1") == "Idle", "Reset to Idle");
    assertTrue(sys.getTotalEscalationsCompleted("anom1") == 1, "1 escalation completed");

    sys.startSite("anom2");
    assertTrue(sys.failEscalation("anom2"), "Fail escalation succeeds");
    assertTrue(sys.getState("anom2") == "Failed", "State is Failed");
    assertTrue(sys.getTotalEscalationsFailed("anom2") == 1, "1 escalation failed");
}

static void testAnomalyEscalationMaxTiers() {
    std::cout << "\n=== AnomalyEscalation: MaxTiers ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    world.createEntity("anom1");
    sys.initialize("anom1", "sys_jita", "player1");

    auto* entity = world.getEntity("anom1");
    auto* comp = entity->getComponent<components::AnomalyEscalation>();
    comp->max_tiers = 2;

    sys.addTier("anom1", 0, "Combat", 1.0f, 1.0f, 5);
    sys.addTier("anom1", 1, "Combat", 1.5f, 1.5f, 8);
    assertTrue(!sys.addTier("anom1", 2, "Combat", 2.0f, 2.0f, 12), "Max tiers enforced");
    assertTrue(sys.getTierCount("anom1") == 2, "Still 2 tiers");
}

static void testAnomalyEscalationMissing() {
    std::cout << "\n=== AnomalyEscalation: Missing ===" << std::endl;
    ecs::World world;
    systems::AnomalyEscalationSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "sys_jita", "player1"), "Init fails on missing");
    assertTrue(!sys.addTier("nonexistent", 0, "Combat", 1.0f, 1.0f, 5), "AddTier fails on missing");
    assertTrue(!sys.startSite("nonexistent"), "Start fails on missing");
    assertTrue(!sys.clearSite("nonexistent", 0.5f), "Clear fails on missing");
    assertTrue(!sys.completeEscalation("nonexistent"), "Complete fails on missing");
    assertTrue(!sys.failEscalation("nonexistent"), "Fail fails on missing");
    assertTrue(sys.getTierCount("nonexistent") == 0, "0 tiers on missing");
    assertTrue(sys.getCurrentTier("nonexistent") == -1, "-1 tier on missing");
    assertTrue(sys.getState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(approxEqual(sys.getDifficultyMultiplier("nonexistent"), 1.0f), "Default difficulty on missing");
    assertTrue(approxEqual(sys.getRewardMultiplier("nonexistent"), 1.0f), "Default reward on missing");
    assertTrue(sys.getTotalSitesCleared("nonexistent") == 0, "0 sites on missing");
    assertTrue(sys.getTotalEscalationsTriggered("nonexistent") == 0, "0 triggered on missing");
    assertTrue(sys.getTotalEscalationsCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(sys.getTotalEscalationsFailed("nonexistent") == 0, "0 failed on missing");
}


void run_anomaly_escalation_system_tests() {
    testAnomalyEscalationCreate();
    testAnomalyEscalationAddTiers();
    testAnomalyEscalationDuplicateTier();
    testAnomalyEscalationStartSite();
    testAnomalyEscalationClearWithEscalation();
    testAnomalyEscalationClearNoEscalation();
    testAnomalyEscalationTimerUpdate();
    testAnomalyEscalationDifficultyReward();
    testAnomalyEscalationCompleteAndFail();
    testAnomalyEscalationMaxTiers();
    testAnomalyEscalationMissing();
}
