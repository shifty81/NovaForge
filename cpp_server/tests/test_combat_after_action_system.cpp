// Tests for: Combat After-Action System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/combat_after_action_system.h"

using namespace atlas;

// ==================== Combat After-Action System Tests ====================

static void testCombatAfterActionCreate() {
    std::cout << "\n=== CombatAfterAction: Create ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getEngagementCount("p1") == 0, "No engagements");
    assertTrue(sys.getCasualtyCount("p1") == 0, "No casualties");
    assertTrue(approxEqual(sys.getTotalDamageDealt("p1"), 0.0), "0 damage dealt");
    assertTrue(approxEqual(sys.getTotalDamageReceived("p1"), 0.0), "0 damage received");
}

static void testCombatAfterActionStartEngagement() {
    std::cout << "\n=== CombatAfterAction: StartEngagement ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.startEngagement("p1", "eng1", "Pirate Frigate"), "Start engagement");
    assertTrue(sys.getEngagementCount("p1") == 1, "1 engagement");
    assertTrue(sys.startEngagement("p1", "eng2", "Pirate Cruiser"), "Start another");
    assertTrue(sys.getEngagementCount("p1") == 2, "2 engagements");
    // Duplicate engagement ID
    assertTrue(!sys.startEngagement("p1", "eng1", "Another Target"), "Duplicate ID rejected");
    assertTrue(sys.getEngagementCount("p1") == 2, "Still 2 engagements");
}

static void testCombatAfterActionEngagementMax() {
    std::cout << "\n=== CombatAfterAction: EngagementMax ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::CombatAfterActionState>();
    state->max_engagements = 2;
    sys.startEngagement("p1", "eng1", "Target A");
    sys.startEngagement("p1", "eng2", "Target B");
    assertTrue(!sys.startEngagement("p1", "eng3", "Target C"), "Max engagements enforced");
    assertTrue(sys.getEngagementCount("p1") == 2, "Still 2");
}

static void testCombatAfterActionRecordHit() {
    std::cout << "\n=== CombatAfterAction: RecordHit ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.startEngagement("p1", "eng1", "Pirate Frigate");
    assertTrue(sys.recordHit("p1", "eng1", 500.0, false), "Outgoing hit");
    assertTrue(sys.recordHit("p1", "eng1", 300.0, false), "Another outgoing hit");
    assertTrue(sys.recordHit("p1", "eng1", 150.0, true), "Incoming hit");
    assertTrue(approxEqual(sys.getEngagementDamageDealt("p1", "eng1"), 800.0), "800 dealt");
    assertTrue(approxEqual(sys.getEngagementDamageReceived("p1", "eng1"), 150.0), "150 received");
    // Unknown engagement
    assertTrue(!sys.recordHit("p1", "unknown", 100.0, false), "Unknown engagement rejected");
}

static void testCombatAfterActionFinalizeEngagement() {
    std::cout << "\n=== CombatAfterAction: FinalizeEngagement ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.startEngagement("p1", "eng1", "Pirate Frigate");
    sys.recordHit("p1", "eng1", 1000.0, false);
    assertTrue(sys.finalizeEngagement("p1", "eng1", 10.0f, 50000.0), "Finalize");
    assertTrue(approxEqual(sys.getEngagementDPS("p1", "eng1"), 100.0), "100 DPS");
    // Double finalize
    assertTrue(!sys.finalizeEngagement("p1", "eng1", 20.0f, 0.0), "Double finalize rejected");
    // Unknown engagement
    assertTrue(!sys.finalizeEngagement("p1", "unknown", 5.0f, 0.0), "Unknown rejected");
}

static void testCombatAfterActionDPS() {
    std::cout << "\n=== CombatAfterAction: DPS ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.startEngagement("p1", "eng1", "Target A");
    sys.recordHit("p1", "eng1", 3000.0, false);
    sys.finalizeEngagement("p1", "eng1", 15.0f, 10000.0);
    assertTrue(approxEqual(sys.getEngagementDPS("p1", "eng1"), 200.0), "200 DPS");
    // Unfinalized engagement (duration 0)
    sys.startEngagement("p1", "eng2", "Target B");
    sys.recordHit("p1", "eng2", 500.0, false);
    assertTrue(approxEqual(sys.getEngagementDPS("p1", "eng2"), 0.0), "0 DPS unfinalized");
    // Unknown engagement
    assertTrue(approxEqual(sys.getEngagementDPS("p1", "unknown"), 0.0), "0 DPS unknown");
}

static void testCombatAfterActionCasualties() {
    std::cout << "\n=== CombatAfterAction: Casualties ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.recordCasualty("p1", "Rifter", 5000000.0), "Record casualty");
    assertTrue(sys.recordCasualty("p1", "Venture", 2000000.0), "Record another");
    assertTrue(sys.getCasualtyCount("p1") == 2, "2 casualties");
    assertTrue(approxEqual(sys.getTotalIskLost("p1"), 7000000.0), "7M ISK lost");
}

static void testCombatAfterActionCasualtyMax() {
    std::cout << "\n=== CombatAfterAction: CasualtyMax ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::CombatAfterActionState>();
    state->max_casualties = 2;
    sys.recordCasualty("p1", "Ship A", 1000.0);
    sys.recordCasualty("p1", "Ship B", 2000.0);
    assertTrue(!sys.recordCasualty("p1", "Ship C", 3000.0), "Max casualties enforced");
    assertTrue(sys.getCasualtyCount("p1") == 2, "Still 2");
}

static void testCombatAfterActionAggregates() {
    std::cout << "\n=== CombatAfterAction: Aggregates ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.startEngagement("p1", "eng1", "Target A");
    sys.recordHit("p1", "eng1", 1000.0, false);
    sys.recordHit("p1", "eng1", 200.0, true);
    sys.finalizeEngagement("p1", "eng1", 10.0f, 50000.0);
    sys.startEngagement("p1", "eng2", "Target B");
    sys.recordHit("p1", "eng2", 2000.0, false);
    sys.recordHit("p1", "eng2", 500.0, true);
    sys.finalizeEngagement("p1", "eng2", 20.0f, 80000.0);
    assertTrue(approxEqual(sys.getTotalDamageDealt("p1"), 3000.0), "3000 total dealt");
    assertTrue(approxEqual(sys.getTotalDamageReceived("p1"), 700.0), "700 total received");
    assertTrue(approxEqual(sys.getTotalIskDestroyed("p1"), 130000.0), "130k ISK destroyed");
    // Average DPS: eng1=100, eng2=100 → avg=100
    assertTrue(approxEqual(sys.getAverageDPS("p1"), 100.0), "100 avg DPS");
}

static void testCombatAfterActionUpdate() {
    std::cout << "\n=== CombatAfterAction: Update ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::CombatAfterActionState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testCombatAfterActionMissing() {
    std::cout << "\n=== CombatAfterAction: Missing ===" << std::endl;
    ecs::World world;
    systems::CombatAfterActionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.startEngagement("nonexistent", "e", "t"), "startEngagement fails");
    assertTrue(!sys.recordHit("nonexistent", "e", 1.0, false), "recordHit fails");
    assertTrue(!sys.finalizeEngagement("nonexistent", "e", 1.0f, 0), "finalize fails");
    assertTrue(sys.getEngagementCount("nonexistent") == 0, "0 engagements");
    assertTrue(approxEqual(sys.getEngagementDamageDealt("nonexistent", "e"), 0.0), "0 dealt");
    assertTrue(approxEqual(sys.getEngagementDamageReceived("nonexistent", "e"), 0.0), "0 received");
    assertTrue(approxEqual(sys.getEngagementDPS("nonexistent", "e"), 0.0), "0 DPS");
    assertTrue(!sys.recordCasualty("nonexistent", "s", 0), "recordCasualty fails");
    assertTrue(sys.getCasualtyCount("nonexistent") == 0, "0 casualties");
    assertTrue(approxEqual(sys.getTotalIskLost("nonexistent"), 0.0), "0 ISK lost");
    assertTrue(approxEqual(sys.getTotalDamageDealt("nonexistent"), 0.0), "0 total dealt");
    assertTrue(approxEqual(sys.getTotalDamageReceived("nonexistent"), 0.0), "0 total received");
    assertTrue(approxEqual(sys.getTotalIskDestroyed("nonexistent"), 0.0), "0 ISK destroyed");
    assertTrue(approxEqual(sys.getAverageDPS("nonexistent"), 0.0), "0 avg DPS");
}

void run_combat_after_action_system_tests() {
    testCombatAfterActionCreate();
    testCombatAfterActionStartEngagement();
    testCombatAfterActionEngagementMax();
    testCombatAfterActionRecordHit();
    testCombatAfterActionFinalizeEngagement();
    testCombatAfterActionDPS();
    testCombatAfterActionCasualties();
    testCombatAfterActionCasualtyMax();
    testCombatAfterActionAggregates();
    testCombatAfterActionUpdate();
    testCombatAfterActionMissing();
}
