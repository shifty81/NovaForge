// Tests for: WarpPerformanceBudgetSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/warp_performance_budget_system.h"

using namespace atlas;

// ==================== WarpPerformanceBudgetSystem Tests ====================

static void testWarpBudgetComputeTotalCost() {
    std::cout << "\n=== WarpPerformanceBudget: ComputeTotalCost ===" << std::endl;
    float costs[5] = {0.2f, 0.3f, 0.1f, 0.15f, 0.05f};
    float total = systems::WarpPerformanceBudgetSystem::computeTotalCost(costs);
    assertTrue(approxEqual(total, 0.8f), "Total cost is 0.8ms");

    float zeros[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    assertTrue(approxEqual(systems::WarpPerformanceBudgetSystem::computeTotalCost(zeros), 0.0f),
               "Zero costs sum to zero");

    // Negative costs clamped to 0
    float negatives[5] = {-0.5f, 0.3f, -0.1f, 0.2f, 0.1f};
    float negTotal = systems::WarpPerformanceBudgetSystem::computeTotalCost(negatives);
    assertTrue(approxEqual(negTotal, 0.6f), "Negative costs treated as 0");
}

static void testWarpBudgetUnderBudget() {
    std::cout << "\n=== WarpPerformanceBudget: UnderBudget ===" << std::endl;
    float costs[5] = {0.2f, 0.2f, 0.2f, 0.2f, 0.2f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    // Total 1.0 <= budget 1.2 — all enabled
    for (int i = 0; i < 5; i++) {
        assertTrue(enabled[i], ("Layer " + std::to_string(i) + " enabled under budget").c_str());
    }
    assertTrue(approxEqual(scale, 1.0f / 1.2f * 1.0f, 0.02f), "Scale factor ~0.83");
    assertTrue(scale > 0.0f && scale <= 1.0f, "Scale in range (0,1]");
}

static void testWarpBudgetExactBudget() {
    std::cout << "\n=== WarpPerformanceBudget: ExactBudget ===" << std::endl;
    float costs[5] = {0.24f, 0.24f, 0.24f, 0.24f, 0.24f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    // Total 1.2 == budget 1.2 — all enabled
    for (int i = 0; i < 5; i++) {
        assertTrue(enabled[i], ("Layer " + std::to_string(i) + " enabled at exact budget").c_str());
    }
    assertTrue(approxEqual(scale, 1.0f), "Scale factor is 1.0 at exact budget");
}

static void testWarpBudgetOverBudget() {
    std::cout << "\n=== WarpPerformanceBudget: OverBudget ===" << std::endl;
    // Costs: 0.5, 0.4, 0.3, 0.2, 0.1 = total 1.5, budget 1.2
    // Highest cost layer (0.5) disabled first → remaining = 1.0
    float costs[5] = {0.5f, 0.4f, 0.3f, 0.2f, 0.1f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    assertTrue(!enabled[0], "Layer 0 (0.5ms) disabled — highest cost");
    assertTrue(enabled[1], "Layer 1 (0.4ms) enabled");
    assertTrue(enabled[2], "Layer 2 (0.3ms) enabled");
    assertTrue(enabled[3], "Layer 3 (0.2ms) enabled");
    assertTrue(enabled[4], "Layer 4 (0.1ms) enabled");

    // Remaining cost: 0.4+0.3+0.2+0.1 = 1.0, scale = 1.0/1.2 ~= 0.833
    assertTrue(approxEqual(scale, 1.0f / 1.2f, 0.02f), "Scale factor after disabling");
}

static void testWarpBudgetMultipleDisabled() {
    std::cout << "\n=== WarpPerformanceBudget: MultipleDisabled ===" << std::endl;
    // Total 2.5, budget 1.0 — need to disable most expensive until under 1.0
    float costs[5] = {0.8f, 0.7f, 0.5f, 0.3f, 0.2f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, 1.0f, enabled);

    // Disable 0.8 → 1.7, still over
    // Disable 0.7 → 1.0, at budget
    assertTrue(!enabled[0], "Layer 0 (0.8ms) disabled");
    assertTrue(!enabled[1], "Layer 1 (0.7ms) disabled");
    assertTrue(enabled[2], "Layer 2 (0.5ms) enabled");
    assertTrue(enabled[3], "Layer 3 (0.3ms) enabled");
    assertTrue(enabled[4], "Layer 4 (0.2ms) enabled");

    // Remaining: 0.5+0.3+0.2 = 1.0, scale = 1.0/1.0 = 1.0
    assertTrue(approxEqual(scale, 1.0f), "Scale factor 1.0 at exactly budget");
}

static void testWarpBudgetZeroBudget() {
    std::cout << "\n=== WarpPerformanceBudget: ZeroBudget ===" << std::endl;
    float costs[5] = {0.2f, 0.3f, 0.1f, 0.15f, 0.05f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, 0.0f, enabled);

    for (int i = 0; i < 5; i++) {
        assertTrue(!enabled[i], ("Layer " + std::to_string(i) + " disabled at zero budget").c_str());
    }
    assertTrue(approxEqual(scale, 0.0f), "Scale factor 0 at zero budget");
}

static void testWarpBudgetNegativeBudget() {
    std::cout << "\n=== WarpPerformanceBudget: NegativeBudget ===" << std::endl;
    float costs[5] = {0.2f, 0.3f, 0.1f, 0.15f, 0.05f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, -1.0f, enabled);

    for (int i = 0; i < 5; i++) {
        assertTrue(!enabled[i], ("Layer " + std::to_string(i) + " disabled at negative budget").c_str());
    }
    assertTrue(approxEqual(scale, 0.0f), "Scale factor 0 at negative budget");
}

static void testWarpBudgetAllZeroCosts() {
    std::cout << "\n=== WarpPerformanceBudget: AllZeroCosts ===" << std::endl;
    float costs[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    bool enabled[5];
    float scale = systems::WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    for (int i = 0; i < 5; i++) {
        assertTrue(enabled[i], ("Layer " + std::to_string(i) + " enabled at zero cost").c_str());
    }
    assertTrue(approxEqual(scale, 0.0f), "Scale factor 0 when no cost");
}

static void testWarpBudgetUpdate() {
    std::cout << "\n=== WarpPerformanceBudget: Update ===" << std::endl;
    ecs::World world;
    systems::WarpPerformanceBudgetSystem sys(&world);
    auto* e = world.createEntity("warp1");
    auto* budget = addComp<components::WarpPerformanceBudget>(e);

    budget->layer_costs[0] = 0.3f;
    budget->layer_costs[1] = 0.2f;
    budget->layer_costs[2] = 0.1f;
    budget->layer_costs[3] = 0.15f;
    budget->layer_costs[4] = 0.05f;
    budget->budget_ms = 1.2f;

    sys.update(0.016f);

    assertTrue(approxEqual(budget->total_cost_ms, 0.8f), "Total cost updated to 0.8ms");
    for (int i = 0; i < 5; i++) {
        assertTrue(budget->layer_enabled[i], ("Layer " + std::to_string(i) + " enabled after update").c_str());
    }
    assertTrue(budget->scale_factor > 0.0f && budget->scale_factor <= 1.0f, "Scale factor in valid range");
}

static void testWarpBudgetUpdateOverBudget() {
    std::cout << "\n=== WarpPerformanceBudget: UpdateOverBudget ===" << std::endl;
    ecs::World world;
    systems::WarpPerformanceBudgetSystem sys(&world);
    auto* e = world.createEntity("warp1");
    auto* budget = addComp<components::WarpPerformanceBudget>(e);

    budget->layer_costs[0] = 0.6f;
    budget->layer_costs[1] = 0.5f;
    budget->layer_costs[2] = 0.4f;
    budget->layer_costs[3] = 0.3f;
    budget->layer_costs[4] = 0.2f;
    budget->budget_ms = 1.0f;

    sys.update(0.016f);

    assertTrue(approxEqual(budget->total_cost_ms, 2.0f), "Total cost updated to 2.0ms");
    // At least one layer should be disabled
    int disabledCount = 0;
    for (int i = 0; i < 5; i++) {
        if (!budget->layer_enabled[i]) disabledCount++;
    }
    assertTrue(disabledCount > 0, "At least one layer disabled when over budget");
    assertTrue(budget->scale_factor >= 0.0f && budget->scale_factor <= 1.0f, "Scale factor in valid range");
}

void run_warp_performance_budget_system_tests() {
    testWarpBudgetComputeTotalCost();
    testWarpBudgetUnderBudget();
    testWarpBudgetExactBudget();
    testWarpBudgetOverBudget();
    testWarpBudgetMultipleDisabled();
    testWarpBudgetZeroBudget();
    testWarpBudgetNegativeBudget();
    testWarpBudgetAllZeroCosts();
    testWarpBudgetUpdate();
    testWarpBudgetUpdateOverBudget();
}
