// Tests for: FPSEnemyAISystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_enemy_ai_system.h"

using namespace atlas;

// ==================== FPSEnemyAISystem Tests ====================

static void testEnemyAISetState() {
    std::cout << "\n=== FPSEnemyAI: Set State ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    addComp<components::FPSEnemyAI>(e);

    assertTrue(sys.getState("guard_1") == 0, "Initial state is Idle");
    assertTrue(sys.setState("guard_1", 1), "Set to Patrol");
    assertTrue(sys.getState("guard_1") == 1, "State is Patrol");
    assertTrue(sys.setState("guard_1", 4), "Set to Attack");
    assertTrue(sys.getState("guard_1") == 4, "State is Attack");
    assertTrue(!sys.setState("guard_1", 7), "Invalid state rejected");
}

static void testEnemyAIPatrol() {
    std::cout << "\n=== FPSEnemyAI: Patrol Route ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    auto* ai = addComp<components::FPSEnemyAI>(e);

    assertTrue(sys.addWaypoint("guard_1", "wp_a", 0.0f, 0.0f, 0.0f, 1.0f), "Add waypoint A");
    assertTrue(sys.addWaypoint("guard_1", "wp_b", 10.0f, 0.0f, 0.0f, 1.0f), "Add waypoint B");
    assertTrue(sys.addWaypoint("guard_1", "wp_c", 10.0f, 10.0f, 0.0f, 1.0f), "Add waypoint C");
    assertTrue(!sys.addWaypoint("guard_1", "wp_a", 0.0f, 0.0f, 0.0f, 1.0f), "Dup rejected");
    assertTrue(sys.getWaypointCount("guard_1") == 3, "3 waypoints");

    sys.setState("guard_1", 1);  // Patrol
    ai->waypoint_wait_timer = 0.0f;  // Force immediate advance
    sys.update(0.1f);  // Should advance to next waypoint
    assertTrue(ai->current_waypoint == 1, "Advanced to waypoint 1");
}

static void testEnemyAIPatrolComplete() {
    std::cout << "\n=== FPSEnemyAI: Patrol Complete ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    auto* ai = addComp<components::FPSEnemyAI>(e);

    sys.addWaypoint("guard_1", "wp_a", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.addWaypoint("guard_1", "wp_b", 10.0f, 0.0f, 0.0f, 0.0f);
    sys.setState("guard_1", 1);

    ai->waypoint_wait_timer = 0.0f;
    sys.update(0.1f);  // wp 0 → 1
    ai->waypoint_wait_timer = 0.0f;
    sys.update(0.1f);  // wp 1 → 0 (loop)

    assertTrue(sys.getTotalPatrolsCompleted("guard_1") == 1, "1 patrol completed");
}

static void testEnemyAIDamage() {
    std::cout << "\n=== FPSEnemyAI: Apply Damage ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    addComp<components::FPSEnemyAI>(e);

    assertTrue(approxEqual(sys.getHealth("guard_1"), 100.0f), "Full health");
    assertTrue(sys.applyDamage("guard_1", 30.0f), "Damage applied");
    assertTrue(approxEqual(sys.getHealth("guard_1"), 70.0f), "Health is 70");
    assertTrue(!sys.applyDamage("guard_1", -10.0f), "Negative damage rejected");
}

static void testEnemyAIDeath() {
    std::cout << "\n=== FPSEnemyAI: Death ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    addComp<components::FPSEnemyAI>(e);

    sys.applyDamage("guard_1", 150.0f);  // Overkill
    assertTrue(sys.isDead("guard_1"), "Guard is dead");
    assertTrue(approxEqual(sys.getHealth("guard_1"), 0.0f), "Health clamped to 0");
    assertTrue(!sys.setState("guard_1", 1), "Can't change dead state");
    assertTrue(!sys.applyDamage("guard_1", 10.0f), "Can't damage dead");
}

static void testEnemyAITarget() {
    std::cout << "\n=== FPSEnemyAI: Target ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    addComp<components::FPSEnemyAI>(e);

    assertTrue(sys.setTarget("guard_1", "player_1"), "Set target");
    assertTrue(sys.getTarget("guard_1") == "player_1", "Target is player_1");
}

static void testEnemyAIDetection() {
    std::cout << "\n=== FPSEnemyAI: Detection ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    auto* ai = addComp<components::FPSEnemyAI>(e);
    ai->detection_range = 20.0f;
    sys.setPosition("guard_1", 0.0f, 0.0f, 0.0f);

    assertTrue(sys.canDetect("guard_1", 10.0f, 0.0f, 0.0f), "Can detect at 10m");
    assertTrue(!sys.canDetect("guard_1", 30.0f, 0.0f, 0.0f), "Can't detect at 30m");

    float dist = sys.getDistanceToTarget("guard_1", 3.0f, 4.0f, 0.0f);
    assertTrue(approxEqual(dist, 5.0f), "Distance is 5 (3-4-5)");
}

static void testEnemyAIAlertDecay() {
    std::cout << "\n=== FPSEnemyAI: Alert Decay ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    auto* ai = addComp<components::FPSEnemyAI>(e);
    ai->alert_duration = 5.0f;

    sys.setState("guard_1", 2);  // Alert
    assertTrue(sys.getState("guard_1") == 2, "State is Alert");

    sys.update(6.0f);  // Alert decays
    assertTrue(sys.getState("guard_1") == 1, "Dropped to Patrol");
}

static void testEnemyAIAttack() {
    std::cout << "\n=== FPSEnemyAI: Attack Ticks ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    auto* ai = addComp<components::FPSEnemyAI>(e);
    ai->attack_cooldown = 1.0f;

    sys.setState("guard_1", 4);  // Attack
    sys.update(0.1f);  // First attack (timer was 0)
    assertTrue(sys.getTotalAttacks("guard_1") == 1, "1 attack");

    sys.update(1.1f);  // Cooldown passed
    assertTrue(sys.getTotalAttacks("guard_1") == 2, "2 attacks");
}

static void testEnemyAIMissingEntity() {
    std::cout << "\n=== FPSEnemyAI: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    assertTrue(!sys.setState("nope", 1), "setState fails");
    assertTrue(sys.getState("nope") == 0, "State is 0");
    assertTrue(!sys.setTarget("nope", "t"), "setTarget fails");
    assertTrue(sys.getTarget("nope") == "", "Target is empty");
    assertTrue(!sys.addWaypoint("nope", "w", 0, 0, 0, 1), "addWaypoint fails");
    assertTrue(!sys.applyDamage("nope", 10), "applyDamage fails");
    assertTrue(approxEqual(sys.getHealth("nope"), 0.0f), "Health is 0");
    assertTrue(!sys.isDead("nope"), "Not dead");
    assertTrue(!sys.isHostile("nope"), "Not hostile");
}

static void testEnemyAIHostile() {
    std::cout << "\n=== FPSEnemyAI: Hostile Check ===" << std::endl;
    ecs::World world;
    systems::FPSEnemyAISystem sys(&world);

    auto* e = world.createEntity("guard_1");
    addComp<components::FPSEnemyAI>(e);

    assertTrue(!sys.isHostile("guard_1"), "Not hostile when Idle");
    sys.setState("guard_1", 1);  // Patrol
    assertTrue(sys.isHostile("guard_1"), "Hostile when Patrol");
    sys.setState("guard_1", 4);  // Attack
    assertTrue(sys.isHostile("guard_1"), "Hostile when Attack");
}

void run_fps_enemy_ai_system_tests() {
    testEnemyAISetState();
    testEnemyAIPatrol();
    testEnemyAIPatrolComplete();
    testEnemyAIDamage();
    testEnemyAIDeath();
    testEnemyAITarget();
    testEnemyAIDetection();
    testEnemyAIAlertDecay();
    testEnemyAIAttack();
    testEnemyAIMissingEntity();
    testEnemyAIHostile();
}
