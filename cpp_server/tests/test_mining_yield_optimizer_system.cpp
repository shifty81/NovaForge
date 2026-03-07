// Tests for: Mining Yield Optimizer System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/mining_yield_optimizer_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mining Yield Optimizer System Tests ====================

static void testMiningYieldOptimizerCreate() {
    std::cout << "\n=== MiningYieldOptimizer: Create ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    assertTrue(sys.initialize("myo1"), "Init succeeds");
    assertTrue(approxEqual(sys.getSkillBonus("myo1"), 0.0f), "Skill bonus is 0");
    assertTrue(approxEqual(sys.getModuleBonus("myo1"), 0.0f), "Module bonus is 0");
    assertTrue(approxEqual(sys.getEnvironmentBonus("myo1"), 0.0f), "Environment bonus is 0");
    assertTrue(approxEqual(sys.getTotalYield("myo1"), 0.0f), "Total yield is 0");
    assertTrue(sys.getCycleCount("myo1") == 0, "0 cycles initially");
}

static void testMiningYieldOptimizerSkillBonus() {
    std::cout << "\n=== MiningYieldOptimizer: SkillBonus ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    assertTrue(sys.setSkillBonus("myo1", 0.25f), "Set skill bonus");
    assertTrue(approxEqual(sys.getSkillBonus("myo1"), 0.25f), "Skill bonus is 25%");
    assertTrue(sys.setSkillBonus("myo1", 0.0f), "Set skill bonus to 0");
    assertTrue(approxEqual(sys.getSkillBonus("myo1"), 0.0f), "Skill bonus is 0%");
}

static void testMiningYieldOptimizerModuleBonus() {
    std::cout << "\n=== MiningYieldOptimizer: ModuleBonus ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    assertTrue(sys.setModuleBonus("myo1", 0.15f), "Set module bonus");
    assertTrue(approxEqual(sys.getModuleBonus("myo1"), 0.15f), "Module bonus is 15%");
}

static void testMiningYieldOptimizerEnvironmentBonus() {
    std::cout << "\n=== MiningYieldOptimizer: EnvironmentBonus ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    assertTrue(sys.setEnvironmentBonus("myo1", 0.10f), "Set environment bonus");
    assertTrue(approxEqual(sys.getEnvironmentBonus("myo1"), 0.10f), "Environment bonus is 10%");
}

static void testMiningYieldOptimizerSecurityModifier() {
    std::cout << "\n=== MiningYieldOptimizer: SecurityModifier ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    assertTrue(sys.setSecurityModifier("myo1", 1.5f), "Set security modifier");
    // Update to recalculate final multiplier
    sys.update(0.1f);
    float mult = sys.getFinalMultiplier("myo1");
    assertTrue(approxEqual(mult, 1.5f), "Final multiplier reflects security (1.0 * 1.0 * 1.0 * 1.5)");
}

static void testMiningYieldOptimizerFinalMultiplier() {
    std::cout << "\n=== MiningYieldOptimizer: FinalMultiplier ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    sys.setSkillBonus("myo1", 0.25f);      // 1.25x
    sys.setModuleBonus("myo1", 0.10f);     // 1.10x
    sys.setEnvironmentBonus("myo1", 0.0f); // 1.00x
    sys.setSecurityModifier("myo1", 1.0f); // 1.00x
    sys.update(0.1f);
    // Expected: 1.25 * 1.10 * 1.00 * 1.00 = 1.375
    float mult = sys.getFinalMultiplier("myo1");
    assertTrue(approxEqual(mult, 1.375f), "Final multiplier is 1.375");
}

static void testMiningYieldOptimizerRecordCycle() {
    std::cout << "\n=== MiningYieldOptimizer: RecordCycle ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    sys.setSkillBonus("myo1", 0.5f);
    sys.update(0.1f);  // recalc multiplier: 1.5 * 1.0 * 1.0 * 1.0 = 1.5
    assertTrue(sys.recordCycle("myo1", 100.0f), "Record cycle with base yield 100");
    assertTrue(approxEqual(sys.getTotalYield("myo1"), 150.0f), "Total yield is 150 (100 * 1.5)");
    assertTrue(sys.getCycleCount("myo1") == 1, "1 cycle recorded");
    assertTrue(sys.recordCycle("myo1", 200.0f), "Record second cycle");
    assertTrue(approxEqual(sys.getTotalYield("myo1"), 450.0f), "Total yield is 450 (150 + 300)");
    assertTrue(sys.getCycleCount("myo1") == 2, "2 cycles recorded");
}

static void testMiningYieldOptimizerAverageYield() {
    std::cout << "\n=== MiningYieldOptimizer: AverageYield ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    sys.update(0.1f);  // multiplier: 1.0
    sys.recordCycle("myo1", 100.0f);  // 100
    sys.recordCycle("myo1", 200.0f);  // 200
    // total: 300, cycles: 2, avg: 150
    assertTrue(approxEqual(sys.getAverageYieldPerCycle("myo1"), 150.0f), "Average yield is 150");
    assertTrue(approxEqual(sys.getAverageYieldPerCycle("nonexistent"), 0.0f), "0 avg on missing");
}

static void testMiningYieldOptimizerNegativeBonus() {
    std::cout << "\n=== MiningYieldOptimizer: NegativeBonus ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    sys.setSkillBonus("myo1", -0.5f);  // should clamp to 0
    assertTrue(approxEqual(sys.getSkillBonus("myo1"), 0.0f), "Negative skill bonus clamped to 0");
    assertTrue(!sys.recordCycle("myo1", -100.0f), "Negative base yield rejected");
}

static void testMiningYieldOptimizerSecurityClamp() {
    std::cout << "\n=== MiningYieldOptimizer: SecurityClamp ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    world.createEntity("myo1");
    sys.initialize("myo1");
    sys.setSecurityModifier("myo1", 5.0f);  // should clamp to 2.0
    sys.update(0.1f);
    float mult = sys.getFinalMultiplier("myo1");
    assertTrue(approxEqual(mult, 2.0f), "Security modifier clamped to 2.0");
    sys.setSecurityModifier("myo1", 0.01f);  // should clamp to 0.1
    sys.update(0.1f);
    mult = sys.getFinalMultiplier("myo1");
    assertTrue(approxEqual(mult, 0.1f), "Security modifier clamped to 0.1");
}

static void testMiningYieldOptimizerMissing() {
    std::cout << "\n=== MiningYieldOptimizer: Missing ===" << std::endl;
    ecs::World world;
    systems::MiningYieldOptimizerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.setSkillBonus("nonexistent", 0.1f), "Skill bonus fails on missing");
    assertTrue(!sys.setModuleBonus("nonexistent", 0.1f), "Module bonus fails on missing");
    assertTrue(!sys.setEnvironmentBonus("nonexistent", 0.1f), "Env bonus fails on missing");
    assertTrue(!sys.setSecurityModifier("nonexistent", 1.0f), "Security modifier fails on missing");
    assertTrue(!sys.recordCycle("nonexistent", 100.0f), "RecordCycle fails on missing");
    assertTrue(approxEqual(sys.getSkillBonus("nonexistent"), 0.0f), "0 skill bonus on missing");
    assertTrue(approxEqual(sys.getModuleBonus("nonexistent"), 0.0f), "0 module bonus on missing");
    assertTrue(approxEqual(sys.getEnvironmentBonus("nonexistent"), 0.0f), "0 env bonus on missing");
    assertTrue(approxEqual(sys.getFinalMultiplier("nonexistent"), 0.0f), "0 multiplier on missing");
    assertTrue(approxEqual(sys.getTotalYield("nonexistent"), 0.0f), "0 total yield on missing");
    assertTrue(sys.getCycleCount("nonexistent") == 0, "0 cycles on missing");
}


void run_mining_yield_optimizer_system_tests() {
    testMiningYieldOptimizerCreate();
    testMiningYieldOptimizerSkillBonus();
    testMiningYieldOptimizerModuleBonus();
    testMiningYieldOptimizerEnvironmentBonus();
    testMiningYieldOptimizerSecurityModifier();
    testMiningYieldOptimizerFinalMultiplier();
    testMiningYieldOptimizerRecordCycle();
    testMiningYieldOptimizerAverageYield();
    testMiningYieldOptimizerNegativeBonus();
    testMiningYieldOptimizerSecurityClamp();
    testMiningYieldOptimizerMissing();
}
