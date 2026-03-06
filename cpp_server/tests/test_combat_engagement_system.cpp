// Tests for: CombatEngagement System Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/combat_engagement_system.h"

using namespace atlas;

// ==================== CombatEngagement System Tests ====================

static void testCombatEngagementCreate() {
    std::cout << "\n=== CombatEngagement: Create ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeEngagement("ship1"), "Init engagement succeeds");
    assertTrue(sys.getState("ship1") == 0, "Starts in Safe state");
    assertTrue(sys.getAttackerCount("ship1") == 0, "No attackers initially");
    assertTrue(!sys.isInCombat("ship1"), "Not in combat initially");
    assertTrue(!sys.isWarpBlocked("ship1"), "Warp not blocked initially");
    assertTrue(!sys.isDockBlocked("ship1"), "Dock not blocked initially");
}

static void testCombatEngagementAddAttacker() {
    std::cout << "\n=== CombatEngagement: AddAttacker ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    assertTrue(sys.addAttacker("ship1", "pirate1"), "Add attacker succeeds");
    assertTrue(sys.getAttackerCount("ship1") == 1, "1 attacker");
    assertTrue(sys.getState("ship1") == 1, "State is Engaging");
}

static void testCombatEngagementDuplicateAttacker() {
    std::cout << "\n=== CombatEngagement: DuplicateAttacker ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    sys.addAttacker("ship1", "pirate1");
    assertTrue(!sys.addAttacker("ship1", "pirate1"), "Duplicate attacker rejected");
    assertTrue(sys.getAttackerCount("ship1") == 1, "Still 1 attacker");
}

static void testCombatEngagementTransitionToInCombat() {
    std::cout << "\n=== CombatEngagement: TransitionToInCombat ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    sys.addAttacker("ship1", "pirate1");
    // After adding attacker, state is Engaging; update transitions to InCombat
    sys.update(1.0f);
    assertTrue(sys.getState("ship1") == 2, "State is InCombat");
    assertTrue(sys.isInCombat("ship1"), "Is in combat");
    assertTrue(sys.isWarpBlocked("ship1"), "Warp blocked in combat");
    assertTrue(sys.isDockBlocked("ship1"), "Dock blocked in combat");
}

static void testCombatEngagementDisengage() {
    std::cout << "\n=== CombatEngagement: Disengage ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    sys.addAttacker("ship1", "pirate1");
    sys.update(1.0f);  // Engaging → InCombat
    sys.removeAttacker("ship1", "pirate1");
    sys.update(1.0f);  // InCombat → Disengaging
    assertTrue(sys.getState("ship1") == 3, "State is Disengaging");
    assertTrue(sys.isWarpBlocked("ship1"), "Warp still blocked while disengaging");
}

static void testCombatEngagementReturnToSafe() {
    std::cout << "\n=== CombatEngagement: ReturnToSafe ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    sys.addAttacker("ship1", "pirate1");
    sys.update(1.0f);  // Engaging → InCombat
    sys.removeAttacker("ship1", "pirate1");
    sys.update(1.0f);  // InCombat → Disengaging
    // Wait out disengage timer (default 15s)
    sys.update(15.0f);
    assertTrue(sys.getState("ship1") == 0, "State is Safe after disengage timer");
    assertTrue(!sys.isWarpBlocked("ship1"), "Warp unblocked when safe");
    assertTrue(!sys.isDockBlocked("ship1"), "Dock unblocked when safe");
}

static void testCombatEngagementReengage() {
    std::cout << "\n=== CombatEngagement: Reengage ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    sys.addAttacker("ship1", "pirate1");
    sys.update(1.0f);  // Engaging → InCombat
    sys.removeAttacker("ship1", "pirate1");
    sys.update(1.0f);  // InCombat → Disengaging
    // Re-engage while disengaging
    sys.addAttacker("ship1", "pirate2");
    sys.update(1.0f);  // Disengaging → InCombat
    assertTrue(sys.getState("ship1") == 2, "Back in combat after re-engage");
}

static void testCombatEngagementSetTarget() {
    std::cout << "\n=== CombatEngagement: SetTarget ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    assertTrue(sys.setTarget("ship1", "enemy1"), "Set target succeeds");
    assertTrue(sys.getState("ship1") == 1, "State is Engaging after setting target");
}

static void testCombatEngagementCombatTime() {
    std::cout << "\n=== CombatEngagement: CombatTime ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeEngagement("ship1");
    sys.addAttacker("ship1", "pirate1");
    sys.update(1.0f);  // Engaging → InCombat
    sys.update(5.0f);  // 5s in combat
    assertTrue(sys.getTotalCombatTime("ship1") > 4.0f, "Combat time accumulated");
    assertTrue(sys.getEngagementCount("ship1") == 1, "1 engagement counted");
}

static void testCombatEngagementMissing() {
    std::cout << "\n=== CombatEngagement: Missing ===" << std::endl;
    ecs::World world;
    systems::CombatEngagementSystem sys(&world);
    assertTrue(!sys.initializeEngagement("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addAttacker("nonexistent", "a1"), "Add attacker fails on missing");
    assertTrue(!sys.removeAttacker("nonexistent", "a1"), "Remove attacker fails on missing");
    assertTrue(!sys.setTarget("nonexistent", "t1"), "Set target fails on missing");
    assertTrue(sys.getAttackerCount("nonexistent") == 0, "0 attackers on missing");
    assertTrue(sys.getState("nonexistent") == 0, "0 state on missing");
    assertTrue(!sys.isInCombat("nonexistent"), "Not in combat on missing");
    assertTrue(!sys.isWarpBlocked("nonexistent"), "Warp not blocked on missing");
    assertTrue(!sys.isDockBlocked("nonexistent"), "Dock not blocked on missing");
    assertTrue(approxEqual(sys.getTimeInState("nonexistent"), 0.0f), "0 time on missing");
    assertTrue(sys.getEngagementCount("nonexistent") == 0, "0 engagements on missing");
    assertTrue(approxEqual(sys.getTotalCombatTime("nonexistent"), 0.0f), "0 combat time on missing");
}

void run_combat_engagement_system_tests() {
    testCombatEngagementCreate();
    testCombatEngagementAddAttacker();
    testCombatEngagementDuplicateAttacker();
    testCombatEngagementTransitionToInCombat();
    testCombatEngagementDisengage();
    testCombatEngagementReturnToSafe();
    testCombatEngagementReengage();
    testCombatEngagementSetTarget();
    testCombatEngagementCombatTime();
    testCombatEngagementMissing();
}
