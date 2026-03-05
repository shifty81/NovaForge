#include "systems/warp_performance_budget_system.h"
#include "components/navigation_components.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

WarpPerformanceBudgetSystem::WarpPerformanceBudgetSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void WarpPerformanceBudgetSystem::updateComponent(ecs::Entity& entity,
    components::WarpPerformanceBudget& budget, float delta_time) {
    float total = computeTotalCost(budget.layer_costs);
    budget.total_cost_ms = total;

    bool enabled[5];
    budget.scale_factor = enforceBudget(budget.layer_costs, budget.budget_ms, enabled);

    for (int i = 0; i < 5; ++i) {
        budget.layer_enabled[i] = enabled[i];
    }
}

} // namespace systems
} // namespace atlas
