// Tests for: ResourceProductionChainSystem Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "components/fleet_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/resource_production_chain_system.h"

using namespace atlas;

// ==================== ResourceProductionChainSystem Tests ====================

static void testProductionChainCreate() {
    std::cout << "\n=== Production Chain: Create ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    assertTrue(sys.createChain("system1", "ore_to_metal"), "Chain created");
    assertTrue(!sys.createChain("system1", "another"), "Duplicate chain rejected");
    assertTrue(!sys.createChain("nonexistent", "chain"), "Missing entity fails");
}

static void testProductionChainAddStage() {
    std::cout << "\n=== Production Chain: Add Stage ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "ore_pipeline");
    assertTrue(sys.addStage("system1", "mining", "asteroid", "ore", 1.0f), "Mining stage added");
    assertTrue(sys.addStage("system1", "refining", "ore", "mineral", 0.8f), "Refining stage added");
    assertTrue(sys.addStage("system1", "manufacturing", "mineral", "product", 0.5f), "Manufacturing stage added");
    assertTrue(!sys.addStage("system1", "mining", "x", "y", 1.0f), "Duplicate stage rejected");
    assertTrue(sys.getStageCount("system1") == 3, "Stage count is 3");
}

static void testProductionChainRemoveStage() {
    std::cout << "\n=== Production Chain: Remove Stage ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "chain1");
    sys.addStage("system1", "mining", "asteroid", "ore", 1.0f);
    sys.addStage("system1", "refining", "ore", "mineral", 0.8f);
    assertTrue(sys.removeStage("system1", "mining"), "Stage removed");
    assertTrue(sys.getStageCount("system1") == 1, "Stage count is 1 after removal");
    assertTrue(!sys.removeStage("system1", "nonexistent"), "Nonexistent stage removal fails");
}

static void testProductionChainEfficiency() {
    std::cout << "\n=== Production Chain: Efficiency ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "chain1");
    sys.addStage("system1", "mining", "asteroid", "ore", 1.0f);
    sys.addStage("system1", "refining", "ore", "mineral", 0.8f);
    sys.setStageEfficiency("system1", "mining", 0.9f);
    sys.setStageEfficiency("system1", "refining", 0.7f);
    sys.update(1.0f);
    // Overall = 0.9 * 0.7 = 0.63
    assertTrue(approxEqual(sys.getOverallEfficiency("system1"), 0.63f), "Overall efficiency is 0.63");
}

static void testProductionChainThroughput() {
    std::cout << "\n=== Production Chain: Throughput ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "chain1");
    sys.addStage("system1", "mining", "asteroid", "ore", 1.0f);
    sys.addStage("system1", "refining", "ore", "mineral", 0.5f);
    sys.update(1.0f);
    // mining: input=1.0, rate=1.0, eff=1.0, bottleneck=0.5 (downstream limited) → 0.5
    // refining: input=0.5, rate=0.5, eff=1.0, bottleneck=1.0 → 0.25
    float output = sys.getTotalOutput("system1");
    assertTrue(output > 0.0f, "Total output is positive");
    assertTrue(approxEqual(sys.getStageThroughput("system1", "refining"), 0.25f), "Refining throughput is 0.25");
}

static void testProductionChainBottleneck() {
    std::cout << "\n=== Production Chain: Bottleneck ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "chain1");
    sys.addStage("system1", "mining", "asteroid", "ore", 2.0f);     // high output
    sys.addStage("system1", "refining", "ore", "mineral", 0.5f);    // low capacity
    sys.update(1.0f);
    // mining bottleneck: downstream capacity (0.5*1.0) / upstream output (2.0*1.0) = 0.25
    float bottleneck = sys.getBottleneckFactor("system1", "mining");
    assertTrue(bottleneck < 1.0f, "Mining stage has bottleneck");
    assertTrue(approxEqual(bottleneck, 0.25f), "Bottleneck factor is 0.25");
}

static void testProductionChainActiveToggle() {
    std::cout << "\n=== Production Chain: Active Toggle ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "chain1");
    assertTrue(sys.isChainActive("system1"), "Chain starts active");
    sys.setChainActive("system1", false);
    assertTrue(!sys.isChainActive("system1"), "Chain deactivated");
    sys.setChainActive("system1", true);
    assertTrue(sys.isChainActive("system1"), "Chain reactivated");
}

static void testProductionChainUptime() {
    std::cout << "\n=== Production Chain: Uptime ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::ResourceProductionChainSystem sys(&world);
    sys.createChain("system1", "chain1");
    sys.addStage("system1", "mining", "asteroid", "ore", 1.0f);
    assertTrue(approxEqual(sys.getUptime("system1"), 0.0f), "Initial uptime is 0");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getUptime("system1"), 5.0f), "Uptime after 5s tick");
    sys.update(3.0f);
    assertTrue(approxEqual(sys.getUptime("system1"), 8.0f), "Uptime after 8s total");
}

static void testProductionChainMissing() {
    std::cout << "\n=== Production Chain: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ResourceProductionChainSystem sys(&world);
    assertTrue(sys.getOverallEfficiency("nonexistent") == 0.0f, "Default efficiency for missing");
    assertTrue(sys.getTotalOutput("nonexistent") == 0.0f, "Default output for missing");
    assertTrue(sys.getStageCount("nonexistent") == 0, "Default stage count for missing");
    assertTrue(!sys.isChainActive("nonexistent"), "Default active for missing");
}

static void testProductionChainComponentDefaults() {
    std::cout << "\n=== Production Chain: Component Defaults ===" << std::endl;
    components::ResourceProductionChain chain;
    assertTrue(chain.stages.empty(), "Default stages empty");
    assertTrue(approxEqual(chain.overall_efficiency, 1.0f), "Default efficiency is 1.0");
    assertTrue(approxEqual(chain.total_output, 0.0f), "Default output is 0.0");
    assertTrue(chain.is_active, "Default is active");
    assertTrue(chain.chain_id.empty(), "Default chain_id empty");
}


void run_resource_production_chain_system_tests() {
    testProductionChainCreate();
    testProductionChainAddStage();
    testProductionChainRemoveStage();
    testProductionChainEfficiency();
    testProductionChainThroughput();
    testProductionChainBottleneck();
    testProductionChainActiveToggle();
    testProductionChainUptime();
    testProductionChainMissing();
    testProductionChainComponentDefaults();
}
