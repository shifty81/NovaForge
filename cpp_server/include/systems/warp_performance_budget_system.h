#ifndef NOVAFORGE_SYSTEMS_WARP_PERFORMANCE_BUDGET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WARP_PERFORMANCE_BUDGET_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

/**
 * WarpPerformanceBudgetSystem — tracks GPU cost per warp visual layer
 * and enforces a total budget (default ≤1.2 ms per frame).
 *
 * Reads WarpPerformanceBudget component (per-layer measured costs).
 * Writes WarpPerformanceBudget::scale_factor and layer enabled flags.
 *
 * Design:
 *   - Each warp layer (radial distortion, starfield bloom, tunnel skin,
 *     vignette, ship silhouette) reports its last-frame GPU time in ms.
 *   - The system sums them and computes a scale_factor (0–1) that shaders
 *     use to reduce resolution / skip layers when over budget.
 *   - Layers are disabled in order of descending cost until within budget.
 *   - No dynamic branching: a single fullscreen pass selects layers via
 *     the enabled mask.
 */
class WarpPerformanceBudgetSystem : public ecs::SingleComponentSystem<components::WarpPerformanceBudget> {
public:
    explicit WarpPerformanceBudgetSystem(ecs::World* world);
    ~WarpPerformanceBudgetSystem() override = default;

    std::string getName() const override { return "WarpPerformanceBudgetSystem"; }

    /**
     * Compute total GPU cost from per-layer costs.
     * @param costs  Array of 5 layer costs in ms
     * @return Sum of all layer costs
     */
    static inline float computeTotalCost(const float costs[5]) {
        float total = 0.0f;
        for (int i = 0; i < 5; ++i) {
            total += (std::max)(costs[i], 0.0f);
        }
        return total;
    }

    /**
     * Determine which layers must be disabled and the resulting scale factor
     * when total cost exceeds the budget.
     *
     * Layers are disabled from most-expensive first until within budget.
     *
     * @param costs       Per-layer GPU cost in ms (5 entries)
     * @param budget_ms   Maximum allowed total cost
     * @param[out] enabled  Per-layer enable flags (5 entries, set to true/false)
     * @return scale_factor (0–1) representing how much of the budget is utilised.
     *         1.0 means at or under budget with all remaining layers active.
     *         <1.0 means layers were scaled/dropped to fit.
     */
    static inline float enforceBudget(const float costs[5], float budget_ms,
                                      bool enabled[5]) {
        if (budget_ms <= 0.0f) {
            for (int i = 0; i < 5; ++i) enabled[i] = false;
            return 0.0f;
        }

        for (int i = 0; i < 5; ++i) enabled[i] = true;

        float total = computeTotalCost(costs);

        if (total <= budget_ms) {
            return (total > 0.0f) ? (std::min)(total / budget_ms, 1.0f) : 0.0f;
        }

        // Over budget — disable layers from most expensive first.
        int order[5] = {0, 1, 2, 3, 4};
        for (int i = 0; i < 4; ++i) {
            for (int j = i + 1; j < 5; ++j) {
                if (costs[order[j]] > costs[order[i]]) {
                    std::swap(order[i], order[j]);
                }
            }
        }

        float running = total;
        for (int k = 0; k < 5 && running > budget_ms; ++k) {
            int idx = order[k];
            running -= (std::max)(costs[idx], 0.0f);
            enabled[idx] = false;
        }

        if (running <= 0.0f) return 0.0f;
        return (std::min)(running / budget_ms, 1.0f);
    }

protected:
    void updateComponent(ecs::Entity& entity, components::WarpPerformanceBudget& budget, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WARP_PERFORMANCE_BUDGET_SYSTEM_H
