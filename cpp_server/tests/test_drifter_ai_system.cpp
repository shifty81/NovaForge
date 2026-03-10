// Tests for: Drifter AI System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/drifter_ai_system.h"

using namespace atlas;

// ==================== Drifter AI System Tests ====================

static void testDrifterAICreate() {
    std::cout << "\n=== DrifterAI: Create ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    assertTrue(sys.initialize("site1", "drifter_hive_01"), "Init succeeds");
    assertTrue(sys.getUnitCount("site1") == 0, "No units initially");
    assertTrue(sys.getAliveCount("site1") == 0, "No alive units");
    assertTrue(sys.getReinforcementWave("site1") == 0, "0 reinforcement waves");
    assertTrue(approxEqual(sys.getDamageTaken("site1"), 0.0f), "0 damage taken");
    assertTrue(sys.getTotalKills("site1") == 0, "0 kills");
    assertTrue(sys.getTotalLosses("site1") == 0, "0 losses");
    assertTrue(!sys.isAreaDenialActive("site1"), "No area denial");
    assertTrue(sys.getThreatLevel("site1") ==
               components::DrifterAIState::ThreatLevel::Passive, "Passive initially");
}

static void testDrifterAIAddUnit() {
    std::cout << "\n=== DrifterAI: AddUnit ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1", "drifter_hive_01");

    assertTrue(sys.addUnit("site1", "d1"), "Add unit 1");
    assertTrue(sys.getUnitCount("site1") == 1, "1 unit");
    assertTrue(sys.getAliveCount("site1") == 1, "1 alive");
    assertTrue(sys.getThreatLevel("site1") ==
               components::DrifterAIState::ThreatLevel::Aggressive, "Aggressive after add");

    assertTrue(sys.addUnit("site1", "d2",
               components::DrifterAIState::DrifterRole::Battleship, 4000.0f, 350.0f),
               "Add battleship");
    assertTrue(sys.getUnitCount("site1") == 2, "2 units");
}

static void testDrifterAIDamageRamp() {
    std::cout << "\n=== DrifterAI: DamageRamp ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    sys.addUnit("site1", "d1");
    // Set a target so ramp engages
    auto* comp = world.getEntity("site1")->getComponent<components::DrifterAIState>();
    comp->units[0].current_target = "player_1";

    // Initial ramp is 1.0
    assertTrue(approxEqual(comp->units[0].ramp_multiplier, 1.0f), "Ramp starts at 1.0");

    // After 5 seconds, ramp should increase (rate=0.1/s → +0.5)
    sys.update(5.0f);
    assertTrue(comp->units[0].ramp_multiplier > 1.0f, "Ramp increased");
    assertTrue(approxEqual(comp->units[0].ramp_multiplier, 1.5f), "Ramp ~1.5 after 5s");

    // Ramp should cap at max (3.0)
    sys.update(20.0f); // 1.5 + 20*0.1 = 3.5, capped at 3.0
    assertTrue(approxEqual(comp->units[0].ramp_multiplier, 3.0f), "Ramp capped at 3.0");
}

static void testDrifterAIApplyDamage() {
    std::cout << "\n=== DrifterAI: ApplyDamage ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");
    sys.addUnit("site1", "d1", components::DrifterAIState::DrifterRole::Cruiser,
                2000.0f, 200.0f);

    assertTrue(sys.applyDamage("site1", "d1", 500.0f), "Damage applied");
    assertTrue(approxEqual(sys.getDamageTaken("site1"), 500.0f), "500 damage tracked");
    assertTrue(sys.getAliveCount("site1") == 1, "Still alive");

    // Kill the unit
    assertTrue(sys.applyDamage("site1", "d1", 1600.0f), "Lethal damage");
    assertTrue(sys.getAliveCount("site1") == 0, "Unit dead");
    assertTrue(sys.getTotalLosses("site1") == 1, "1 loss");
}

static void testDrifterAIBerserk() {
    std::cout << "\n=== DrifterAI: Berserk ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");
    sys.addUnit("site1", "d1", components::DrifterAIState::DrifterRole::Cruiser,
                5000.0f, 200.0f);

    // Provocation threshold is 3000
    sys.applyDamage("site1", "d1", 2000.0f);
    assertTrue(sys.getThreatLevel("site1") !=
               components::DrifterAIState::ThreatLevel::Berserk, "Not berserk yet");

    sys.applyDamage("site1", "d1", 1500.0f);
    assertTrue(sys.getThreatLevel("site1") ==
               components::DrifterAIState::ThreatLevel::Berserk, "Berserk at 3500 damage");
}

static void testDrifterAIReinforcements() {
    std::cout << "\n=== DrifterAI: Reinforcements ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");
    sys.addUnit("site1", "d1", components::DrifterAIState::DrifterRole::Cruiser,
                10000.0f, 200.0f);

    // Trigger berserk
    sys.applyDamage("site1", "d1", 3500.0f);
    assertTrue(sys.getThreatLevel("site1") ==
               components::DrifterAIState::ThreatLevel::Berserk, "Berserk triggered");
    assertTrue(sys.getReinforcementWave("site1") == 0, "No reinforcements yet");

    // Tick past reinforcement cooldown (45s)
    sys.update(46.0f);
    assertTrue(sys.getReinforcementWave("site1") == 1, "Wave 1 arrived");
    assertTrue(sys.getUnitCount("site1") == 3, "3 units (1 original + 2 reinforcements)");

    // Second wave
    sys.update(46.0f);
    assertTrue(sys.getReinforcementWave("site1") == 2, "Wave 2 arrived");
    assertTrue(sys.getUnitCount("site1") == 5, "5 units total");

    // No more waves (max 2)
    sys.update(46.0f);
    assertTrue(sys.getReinforcementWave("site1") == 2, "Capped at 2 waves");
}

static void testDrifterAIAreaDenial() {
    std::cout << "\n=== DrifterAI: AreaDenial ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    assertTrue(!sys.isAreaDenialActive("site1"), "Not active initially");
    assertTrue(sys.activateAreaDenial("site1"), "Activate area denial");
    assertTrue(sys.isAreaDenialActive("site1"), "Area denial active");

    // Tick past duration (30s)
    sys.update(31.0f);
    assertTrue(!sys.isAreaDenialActive("site1"), "Area denial expired");
}

static void testDrifterAIMaxUnits() {
    std::cout << "\n=== DrifterAI: MaxUnits ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    world.createEntity("site1");
    sys.initialize("site1");

    for (int i = 0; i < 8; i++) {
        assertTrue(sys.addUnit("site1", "d" + std::to_string(i)),
                   "Add unit " + std::to_string(i));
    }
    assertTrue(sys.getUnitCount("site1") == 8, "8 units (max)");
    assertTrue(!sys.addUnit("site1", "overflow"), "Overflow rejected");
}

static void testDrifterAIMissing() {
    std::cout << "\n=== DrifterAI: Missing ===" << std::endl;
    ecs::World world;
    systems::DrifterAISystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addUnit("nonexistent", "d1"), "Add fails on missing");
    assertTrue(!sys.removeUnit("nonexistent", "d1"), "Remove fails on missing");
    assertTrue(!sys.applyDamage("nonexistent", "d1", 100.0f), "Damage fails on missing");
    assertTrue(!sys.setThreatLevel("nonexistent",
               components::DrifterAIState::ThreatLevel::Berserk), "SetThreat fails on missing");
    assertTrue(!sys.activateAreaDenial("nonexistent"), "AreaDenial fails on missing");
    assertTrue(sys.getUnitCount("nonexistent") == 0, "0 units on missing");
    assertTrue(sys.getAliveCount("nonexistent") == 0, "0 alive on missing");
    assertTrue(sys.getReinforcementWave("nonexistent") == 0, "0 waves on missing");
    assertTrue(approxEqual(sys.getDamageTaken("nonexistent"), 0.0f), "0 damage on missing");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(sys.getTotalLosses("nonexistent") == 0, "0 losses on missing");
    assertTrue(!sys.isAreaDenialActive("nonexistent"), "No area denial on missing");
    assertTrue(sys.getThreatLevel("nonexistent") ==
               components::DrifterAIState::ThreatLevel::Passive, "Passive on missing");
}

void run_drifter_ai_system_tests() {
    testDrifterAICreate();
    testDrifterAIAddUnit();
    testDrifterAIDamageRamp();
    testDrifterAIApplyDamage();
    testDrifterAIBerserk();
    testDrifterAIReinforcements();
    testDrifterAIAreaDenial();
    testDrifterAIMaxUnits();
    testDrifterAIMissing();
}
