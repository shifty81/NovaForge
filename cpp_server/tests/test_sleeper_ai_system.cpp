// Tests for: Sleeper AI System
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/sleeper_ai_system.h"

using namespace atlas;

// ==================== Sleeper AI System Tests ====================

static void testSleeperAICreate() {
    std::cout << "\n=== SleeperAI: Create ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    assertTrue(sys.initialize("site1", "wh_site_alpha"), "Init succeeds");
    assertTrue(sys.getUnitCount("site1") == 0, "No units initially");
    assertTrue(sys.getAliveCount("site1") == 0, "No alive units");
    assertTrue(sys.getEscalationWave("site1") == 0, "Wave 0");
    assertTrue(approxEqual(sys.getDamageTaken("site1"), 0.0f), "0 damage");
    assertTrue(sys.getTotalKills("site1") == 0, "0 kills");
    assertTrue(sys.getTotalLosses("site1") == 0, "0 losses");
    assertTrue(sys.getAlertLevel("site1") == components::SleeperAIState::AlertLevel::Dormant,
               "Dormant initially");
}

static void testSleeperAIAddUnits() {
    std::cout << "\n=== SleeperAI: AddUnits ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    assertTrue(sys.addUnit("site1", "sentry_01",
        components::SleeperAIState::SleeperRole::Sentry, 1000.0f, 150.0f),
        "Add sentry");
    assertTrue(sys.addUnit("site1", "guardian_01",
        components::SleeperAIState::SleeperRole::Guardian, 800.0f, 50.0f, 100.0f),
        "Add guardian");
    assertTrue(sys.getUnitCount("site1") == 2, "2 units");
    assertTrue(sys.getAliveCount("site1") == 2, "2 alive");
    // Adding a unit wakes from dormant
    assertTrue(sys.getAlertLevel("site1") == components::SleeperAIState::AlertLevel::Alerted,
               "Alerted after adding units");
}

static void testSleeperAIRemoveUnit() {
    std::cout << "\n=== SleeperAI: RemoveUnit ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");
    sys.addUnit("site1", "sentry_01");
    sys.addUnit("site1", "sentry_02");

    assertTrue(sys.removeUnit("site1", "sentry_01"), "Remove succeeds");
    assertTrue(sys.getUnitCount("site1") == 1, "1 unit remaining");
    assertTrue(!sys.removeUnit("site1", "sentry_01"), "Double remove fails");
}

static void testSleeperAIDamage() {
    std::cout << "\n=== SleeperAI: Damage ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");
    sys.addUnit("site1", "sentry_01",
        components::SleeperAIState::SleeperRole::Sentry, 500.0f, 150.0f);

    assertTrue(sys.applyDamage("site1", "sentry_01", 200.0f), "Apply 200 damage");
    assertTrue(approxEqual(sys.getDamageTaken("site1"), 200.0f), "200 total damage");
    assertTrue(sys.getAlertLevel("site1") == components::SleeperAIState::AlertLevel::Combat,
               "Combat alert after damage");

    // Kill the unit
    assertTrue(sys.applyDamage("site1", "sentry_01", 400.0f), "Apply lethal damage");
    assertTrue(sys.getTotalLosses("site1") == 1, "1 loss");
    assertTrue(sys.getAliveCount("site1") == 0, "0 alive");
    assertTrue(!sys.applyDamage("site1", "sentry_01", 100.0f), "Can't damage dead unit");
}

static void testSleeperAIEscalation() {
    std::cout << "\n=== SleeperAI: Escalation ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    // Add units and set to combat
    sys.addUnit("site1", "sentry_01",
        components::SleeperAIState::SleeperRole::Sentry, 5000.0f, 150.0f);
    sys.setAlertLevel("site1", components::SleeperAIState::AlertLevel::Combat);

    // Apply enough damage to trigger escalation (threshold = 2000)
    sys.applyDamage("site1", "sentry_01", 2500.0f);

    // Need to tick past escalation cooldown (60s)
    sys.update(61.0f);
    assertTrue(sys.getEscalationWave("site1") == 1, "Escalation wave 1");
    assertTrue(sys.getAlertLevel("site1") == components::SleeperAIState::AlertLevel::Escalated,
               "Escalated alert level");
    // Reinforcements spawned (2 units per wave)
    assertTrue(sys.getUnitCount("site1") == 3, "3 units after reinforcement");
}

static void testSleeperAIMaxUnits() {
    std::cout << "\n=== SleeperAI: MaxUnits ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    for (int i = 0; i < 10; i++) {
        assertTrue(sys.addUnit("site1", "unit_" + std::to_string(i)),
                   "Add unit " + std::to_string(i));
    }
    assertTrue(sys.getUnitCount("site1") == 10, "10 units (max)");
    assertTrue(!sys.addUnit("site1", "overflow"), "Overflow rejected");
}

static void testSleeperAISetAlertLevel() {
    std::cout << "\n=== SleeperAI: SetAlertLevel ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    assertTrue(sys.setAlertLevel("site1", components::SleeperAIState::AlertLevel::Combat),
               "Set to Combat");
    assertTrue(sys.getAlertLevel("site1") == components::SleeperAIState::AlertLevel::Combat,
               "Alert is Combat");
}

static void testSleeperAIMissing() {
    std::cout << "\n=== SleeperAI: Missing ===" << std::endl;
    ecs::World world;
    systems::SleeperAISystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addUnit("nonexistent", "u1"), "Add fails on missing");
    assertTrue(!sys.removeUnit("nonexistent", "u1"), "Remove fails on missing");
    assertTrue(!sys.applyDamage("nonexistent", "u1", 100.0f), "Damage fails on missing");
    assertTrue(!sys.setAlertLevel("nonexistent",
        components::SleeperAIState::AlertLevel::Combat), "SetAlert fails on missing");
    assertTrue(sys.getUnitCount("nonexistent") == 0, "0 units on missing");
    assertTrue(sys.getAliveCount("nonexistent") == 0, "0 alive on missing");
    assertTrue(sys.getEscalationWave("nonexistent") == 0, "0 waves on missing");
    assertTrue(approxEqual(sys.getDamageTaken("nonexistent"), 0.0f), "0 damage on missing");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(sys.getTotalLosses("nonexistent") == 0, "0 losses on missing");
    assertTrue(sys.getAlertLevel("nonexistent") == components::SleeperAIState::AlertLevel::Dormant,
               "Dormant on missing");
}

void run_sleeper_ai_system_tests() {
    testSleeperAICreate();
    testSleeperAIAddUnits();
    testSleeperAIRemoveUnit();
    testSleeperAIDamage();
    testSleeperAIEscalation();
    testSleeperAIMaxUnits();
    testSleeperAISetAlertLevel();
    testSleeperAIMissing();
}
