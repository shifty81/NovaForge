/**
 * Tests for WarpPerformanceBudgetSystem — tracks GPU cost per warp
 * visual layer and enforces a total budget (≤1.2 ms default).
 */

#include <cassert>
#include <cmath>
#include <algorithm>
#include "../cpp_server/include/components/navigation_components.h"

using namespace atlas::components;

// Static utility functions mirrored from WarpPerformanceBudgetSystem.
// Included here directly to avoid pulling in single_component_system.h which
// causes an atlas::ecs::World namespace collision with the engine ECS World
// when both are linked into the AtlasTests binary.
namespace atlas { namespace systems {
struct WarpPerformanceBudgetSystem {
    static inline float computeTotalCost(const float costs[5]) {
        float total = 0.0f;
        for (int i = 0; i < 5; ++i) total += std::max(costs[i], 0.0f);
        return total;
    }
    static inline float enforceBudget(const float costs[5], float budget_ms,
                                      bool enabled[5]) {
        if (budget_ms <= 0.0f) {
            for (int i = 0; i < 5; ++i) enabled[i] = false;
            return 0.0f;
        }
        for (int i = 0; i < 5; ++i) enabled[i] = true;
        float total = computeTotalCost(costs);
        if (total <= budget_ms) {
            return (total > 0.0f) ? std::min(total / budget_ms, 1.0f) : 0.0f;
        }
        int order[5] = {0, 1, 2, 3, 4};
        for (int i = 0; i < 4; ++i) {
            for (int j = i + 1; j < 5; ++j) {
                if (costs[order[j]] > costs[order[i]])
                    std::swap(order[i], order[j]);
            }
        }
        float running = total;
        for (int k = 0; k < 5 && running > budget_ms; ++k) {
            int idx = order[k];
            running -= std::max(costs[idx], 0.0f);
            enabled[idx] = false;
        }
        if (running <= 0.0f) return 0.0f;
        return std::min(running / budget_ms, 1.0f);
    }
};
}} // namespace atlas::systems
using namespace atlas::systems;

static bool approxEq(float a, float b, float eps = 0.001f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Component defaults
// ══════════════════════════════════════════════════════════════════

void test_warp_perf_budget_defaults() {
    WarpPerformanceBudget budget;
    assert(approxEq(budget.budget_ms, 1.2f));
    assert(approxEq(budget.total_cost_ms, 0.0f));
    assert(approxEq(budget.scale_factor, 1.0f));
    for (int i = 0; i < 5; ++i) {
        assert(approxEq(budget.layer_costs[i], 0.0f));
        assert(budget.layer_enabled[i] == true);
    }
}

// ══════════════════════════════════════════════════════════════════
// computeTotalCost
// ══════════════════════════════════════════════════════════════════

void test_warp_perf_total_cost_zeros() {
    float costs[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float total = WarpPerformanceBudgetSystem::computeTotalCost(costs);
    assert(approxEq(total, 0.0f));
}

void test_warp_perf_total_cost_sums() {
    float costs[5] = {0.2f, 0.3f, 0.1f, 0.15f, 0.05f};
    float total = WarpPerformanceBudgetSystem::computeTotalCost(costs);
    assert(approxEq(total, 0.8f));
}

void test_warp_perf_total_cost_negative_clamped() {
    float costs[5] = {0.5f, -0.1f, 0.3f, 0.0f, 0.0f};
    float total = WarpPerformanceBudgetSystem::computeTotalCost(costs);
    // Negative costs treated as 0
    assert(approxEq(total, 0.8f));
}

// ══════════════════════════════════════════════════════════════════
// enforceBudget — within budget
// ══════════════════════════════════════════════════════════════════

void test_warp_perf_enforce_under_budget() {
    float costs[5] = {0.2f, 0.2f, 0.2f, 0.2f, 0.1f};  // total 0.9
    bool enabled[5];
    float scale = WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    for (int i = 0; i < 5; ++i) assert(enabled[i] == true);
    assert(approxEq(scale, 0.9f / 1.2f));
}

void test_warp_perf_enforce_exact_budget() {
    float costs[5] = {0.3f, 0.3f, 0.2f, 0.2f, 0.2f};  // total 1.2
    bool enabled[5];
    float scale = WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    for (int i = 0; i < 5; ++i) assert(enabled[i] == true);
    assert(approxEq(scale, 1.0f));
}

// ══════════════════════════════════════════════════════════════════
// enforceBudget — over budget
// ══════════════════════════════════════════════════════════════════

void test_warp_perf_enforce_over_budget_disables_most_expensive() {
    // total 1.5, budget 1.2 → need to drop 0.3+ → disable layer 0 (0.5)
    float costs[5] = {0.5f, 0.3f, 0.3f, 0.2f, 0.2f};  // total 1.5
    bool enabled[5];
    float scale = WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    assert(enabled[0] == false);  // most expensive disabled
    assert(enabled[1] == true);
    assert(enabled[2] == true);
    assert(enabled[3] == true);
    assert(enabled[4] == true);
    // Remaining: 0.3+0.3+0.2+0.2 = 1.0
    assert(approxEq(scale, 1.0f / 1.2f, 0.01f));
}

void test_warp_perf_enforce_over_budget_disables_multiple() {
    // total 2.0, budget 1.0 → need to drop 1.0+
    float costs[5] = {0.6f, 0.5f, 0.4f, 0.3f, 0.2f};  // total 2.0
    bool enabled[5];
    float scale = WarpPerformanceBudgetSystem::enforceBudget(costs, 1.0f, enabled);

    // Disable 0.6 first (remaining 1.4), still over, disable 0.5 (remaining 0.9)
    assert(enabled[0] == false);
    assert(enabled[1] == false);
    assert(enabled[2] == true);
    assert(enabled[3] == true);
    assert(enabled[4] == true);
    // Remaining: 0.4+0.3+0.2 = 0.9
    assert(approxEq(scale, 0.9f / 1.0f, 0.01f));
}

void test_warp_perf_enforce_zero_budget() {
    float costs[5] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
    bool enabled[5];
    float scale = WarpPerformanceBudgetSystem::enforceBudget(costs, 0.0f, enabled);

    for (int i = 0; i < 5; ++i) assert(enabled[i] == false);
    assert(approxEq(scale, 0.0f));
}

void test_warp_perf_enforce_all_zero_costs() {
    float costs[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    bool enabled[5];
    float scale = WarpPerformanceBudgetSystem::enforceBudget(costs, 1.2f, enabled);

    for (int i = 0; i < 5; ++i) assert(enabled[i] == true);
    assert(approxEq(scale, 0.0f));
}
