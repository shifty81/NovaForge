// Tests for: Aggression Switching System
#include "test_log.h"
#include "components/combat_components.h"
#include "systems/aggression_switching_system.h"

using namespace atlas;

// ==================== Aggression Switching System Tests ====================

static void testAggressionCreate() {
    std::cout << "\n=== AggressionSwitching: Create ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    assertTrue(sys.initialize("npc1"), "Init succeeds");
    assertTrue(sys.getThreatSourceCount("npc1") == 0, "No threat sources initially");
    assertTrue(sys.getCurrentTarget("npc1").empty(), "No current target");
    assertTrue(sys.getTotalSwitches("npc1") == 0, "0 switches");
    assertTrue(!sys.isLocked("npc1"), "Not locked by default");
}

static void testAggressionAddThreat() {
    std::cout << "\n=== AggressionSwitching: AddThreat ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    assertTrue(sys.addThreatSource("npc1", "player1", 100.0f, 0.0f, 0.0f), "Add player1");
    assertTrue(sys.getThreatSourceCount("npc1") == 1, "1 source");
    assertTrue(sys.getCurrentTarget("npc1") == "player1", "Auto-assigned first target");
    assertTrue(sys.addThreatSource("npc1", "player2", 50.0f, 0.0f, 0.0f), "Add player2");
    assertTrue(sys.getThreatSourceCount("npc1") == 2, "2 sources");
    assertTrue(!sys.addThreatSource("npc1", "player1", 10.0f, 0.0f, 0.0f), "Duplicate rejected");
    assertTrue(sys.getThreatSourceCount("npc1") == 2, "Still 2 sources");
}

static void testAggressionRemoveThreat() {
    std::cout << "\n=== AggressionSwitching: RemoveThreat ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    sys.addThreatSource("npc1", "player1", 100.0f);
    assertTrue(sys.removeThreatSource("npc1", "player1"), "Remove succeeds");
    assertTrue(sys.getThreatSourceCount("npc1") == 0, "0 sources after remove");
    assertTrue(sys.getCurrentTarget("npc1").empty(), "Target cleared after remove");
    assertTrue(!sys.removeThreatSource("npc1", "player1"), "Double remove fails");
}

static void testAggressionDamage() {
    std::cout << "\n=== AggressionSwitching: ApplyDamage ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");
    sys.addThreatSource("npc1", "player1", 50.0f);
    assertTrue(sys.applyDamage("npc1", "player1", 200.0f), "Apply damage succeeds");
    assertTrue(approxEqual(sys.getThreatFor("npc1", "player1"), 250.0f), "Threat accumulated");
    assertTrue(!sys.applyDamage("npc1", "unknown", 100.0f), "Damage to unknown fails");
}

static void testAggressionSwitchTarget() {
    std::cout << "\n=== AggressionSwitching: SwitchTarget ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* state = entity->getComponent<components::AggressionSwitchingState>();
    state->evaluation_interval = 1.0f;
    state->switch_threshold = 1.2f;

    sys.addThreatSource("npc1", "player1", 100.0f);
    sys.addThreatSource("npc1", "player2", 50.0f);
    assertTrue(sys.getCurrentTarget("npc1") == "player1", "Initial target is player1");

    // player2 gets massive threat boost
    sys.applyDamage("npc1", "player2", 200.0f);
    // trigger evaluation
    sys.update(1.1f);
    assertTrue(sys.getCurrentTarget("npc1") == "player2", "Switched to player2 after threat surge");
    assertTrue(sys.getTotalSwitches("npc1") == 1, "1 switch recorded");
}

static void testAggressionHysteresis() {
    std::cout << "\n=== AggressionSwitching: Hysteresis ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* state = entity->getComponent<components::AggressionSwitchingState>();
    state->evaluation_interval = 1.0f;
    state->switch_threshold = 1.5f;  // 50% threshold

    sys.addThreatSource("npc1", "player1", 100.0f);
    sys.addThreatSource("npc1", "player2", 120.0f);
    // player2 is higher but not 50% higher → no switch
    sys.update(1.1f);
    assertTrue(sys.getCurrentTarget("npc1") == "player1", "No switch due to hysteresis");
    assertTrue(sys.getTotalSwitches("npc1") == 0, "0 switches");
}

static void testAggressionLocked() {
    std::cout << "\n=== AggressionSwitching: Locked ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* state = entity->getComponent<components::AggressionSwitchingState>();
    state->evaluation_interval = 1.0f;

    sys.addThreatSource("npc1", "player1", 100.0f);
    sys.addThreatSource("npc1", "player2", 500.0f);
    assertTrue(sys.setLocked("npc1", true), "Lock succeeds");
    assertTrue(sys.isLocked("npc1"), "Is locked");
    sys.update(1.1f);
    assertTrue(sys.getCurrentTarget("npc1") == "player1", "Target unchanged while locked");
    assertTrue(sys.getTotalSwitches("npc1") == 0, "No switches while locked");
}

static void testAggressionDecay() {
    std::cout << "\n=== AggressionSwitching: ThreatDecay ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    world.createEntity("npc1");
    sys.initialize("npc1");

    auto* entity = world.getEntity("npc1");
    auto* state = entity->getComponent<components::AggressionSwitchingState>();
    state->threat_decay_rate = 10.0f;  // fast decay for test

    sys.addThreatSource("npc1", "player1", 50.0f);
    // After 5 seconds, threat should decay by 50 (10 * 5)
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getThreatFor("npc1", "player1"), 0.0f), "Threat decayed to 0");
}

static void testAggressionMissing() {
    std::cout << "\n=== AggressionSwitching: Missing ===" << std::endl;
    ecs::World world;
    systems::AggressionSwitchingSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addThreatSource("nonexistent", "t1"), "Add threat fails on missing");
    assertTrue(!sys.removeThreatSource("nonexistent", "t1"), "Remove threat fails on missing");
    assertTrue(!sys.applyDamage("nonexistent", "t1", 100.0f), "Apply damage fails on missing");
    assertTrue(sys.getThreatSourceCount("nonexistent") == 0, "0 sources on missing");
    assertTrue(approxEqual(sys.getThreatFor("nonexistent", "t1"), 0.0f), "0 threat on missing");
    assertTrue(sys.getCurrentTarget("nonexistent").empty(), "Empty target on missing");
    assertTrue(sys.getTotalSwitches("nonexistent") == 0, "0 switches on missing");
    assertTrue(!sys.setLocked("nonexistent", true), "SetLocked fails on missing");
    assertTrue(!sys.isLocked("nonexistent"), "Not locked on missing");
}

void run_aggression_switching_system_tests() {
    testAggressionCreate();
    testAggressionAddThreat();
    testAggressionRemoveThreat();
    testAggressionDamage();
    testAggressionSwitchTarget();
    testAggressionHysteresis();
    testAggressionLocked();
    testAggressionDecay();
    testAggressionMissing();
}
