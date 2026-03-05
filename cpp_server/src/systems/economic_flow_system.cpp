#include "systems/economic_flow_system.h"
#include "components/economy_components.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

EconomicFlowSystem::EconomicFlowSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void EconomicFlowSystem::updateComponent(ecs::Entity& /*entity*/, components::EconomicFlowState& flow, float delta_time) {
    // Sum production rates
    float sum_production = 0.0f;
    for (const auto& pair : flow.production_rate) {
        sum_production += pair.second;
    }
    flow.total_production = sum_production;

    // Sum consumption rates
    float sum_consumption = 0.0f;
    for (const auto& pair : flow.consumption_rate) {
        sum_consumption += pair.second;
    }
    flow.total_consumption = sum_consumption;

    // Compute economic health
    float computed_health;
    if (sum_consumption > 0.0f) {
        computed_health = sum_production / sum_consumption;
    } else if (sum_production > 0.0f) {
        computed_health = 2.0f;
    } else {
        computed_health = 1.0f;
    }
    computed_health = std::clamp(computed_health, 0.0f, 2.0f);

    // Smooth health transition
    flow.economic_health += (computed_health - flow.economic_health) * health_smoothing;

    // Decay all rates towards zero
    float decay = 1.0f - flow_decay_rate * delta_time;
    decay = std::clamp(decay, 0.0f, 1.0f);

    for (auto& pair : flow.production_rate)    pair.second *= decay;
    for (auto& pair : flow.consumption_rate)   pair.second *= decay;
    for (auto& pair : flow.transport_in_rate)   pair.second *= decay;
    for (auto& pair : flow.transport_out_rate)  pair.second *= decay;
    for (auto& pair : flow.destruction_rate)    pair.second *= decay;

    flow.last_update_time += delta_time;
}

// -----------------------------------------------------------------------
// Recording API
// -----------------------------------------------------------------------

void EconomicFlowSystem::recordProduction(const std::string& system_id,
                                           const std::string& commodity,
                                           float amount) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;

    auto* flow = entity->getComponent<components::EconomicFlowState>();
    if (!flow) {
        entity->addComponent(std::make_unique<components::EconomicFlowState>());
        flow = entity->getComponent<components::EconomicFlowState>();
    }
    if (!flow) return;

    flow->production_rate[commodity] += amount;
}

void EconomicFlowSystem::recordConsumption(const std::string& system_id,
                                            const std::string& commodity,
                                            float amount) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;

    auto* flow = entity->getComponent<components::EconomicFlowState>();
    if (!flow) {
        entity->addComponent(std::make_unique<components::EconomicFlowState>());
        flow = entity->getComponent<components::EconomicFlowState>();
    }
    if (!flow) return;

    flow->consumption_rate[commodity] += amount;
}

void EconomicFlowSystem::recordTransport(const std::string& from_system,
                                          const std::string& to_system,
                                          const std::string& commodity,
                                          float amount) {
    // Record outflow on source system
    auto* from_entity = world_->getEntity(from_system);
    if (from_entity) {
        auto* from_flow = from_entity->getComponent<components::EconomicFlowState>();
        if (!from_flow) {
            from_entity->addComponent(std::make_unique<components::EconomicFlowState>());
            from_flow = from_entity->getComponent<components::EconomicFlowState>();
        }
        if (from_flow) {
            from_flow->transport_out_rate[commodity] += amount;
        }
    }

    // Record inflow on destination system
    auto* to_entity = world_->getEntity(to_system);
    if (to_entity) {
        auto* to_flow = to_entity->getComponent<components::EconomicFlowState>();
        if (!to_flow) {
            to_entity->addComponent(std::make_unique<components::EconomicFlowState>());
            to_flow = to_entity->getComponent<components::EconomicFlowState>();
        }
        if (to_flow) {
            to_flow->transport_in_rate[commodity] += amount;
        }
    }
}

void EconomicFlowSystem::recordDestruction(const std::string& system_id,
                                            const std::string& commodity,
                                            float amount) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;

    auto* flow = entity->getComponent<components::EconomicFlowState>();
    if (!flow) {
        entity->addComponent(std::make_unique<components::EconomicFlowState>());
        flow = entity->getComponent<components::EconomicFlowState>();
    }
    if (!flow) return;

    flow->destruction_rate[commodity] += amount;
}

// -----------------------------------------------------------------------
// Query API
// -----------------------------------------------------------------------

float EconomicFlowSystem::getEconomicHealth(const std::string& system_id) const {
    const auto* flow = getComponentFor(system_id);
    if (!flow) return 1.0f;

    return flow->economic_health;
}

float EconomicFlowSystem::getProductionRate(const std::string& system_id,
                                             const std::string& commodity) const {
    const auto* flow = getComponentFor(system_id);
    if (!flow) return 0.0f;

    auto it = flow->production_rate.find(commodity);
    return (it != flow->production_rate.end()) ? it->second : 0.0f;
}

float EconomicFlowSystem::getConsumptionRate(const std::string& system_id,
                                              const std::string& commodity) const {
    const auto* flow = getComponentFor(system_id);
    if (!flow) return 0.0f;

    auto it = flow->consumption_rate.find(commodity);
    return (it != flow->consumption_rate.end()) ? it->second : 0.0f;
}

float EconomicFlowSystem::getNetFlow(const std::string& system_id,
                                      const std::string& commodity) const {
    const auto* flow = getComponentFor(system_id);
    if (!flow) return 0.0f;

    auto getRate = [](const std::map<std::string, float>& rates, const std::string& key) -> float {
        auto it = rates.find(key);
        return (it != rates.end()) ? it->second : 0.0f;
    };

    return getRate(flow->production_rate, commodity)
         + getRate(flow->transport_in_rate, commodity)
         - getRate(flow->consumption_rate, commodity)
         - getRate(flow->transport_out_rate, commodity)
         - getRate(flow->destruction_rate, commodity);
}

float EconomicFlowSystem::getTotalProduction(const std::string& system_id) const {
    const auto* flow = getComponentFor(system_id);
    if (!flow) return 0.0f;

    return flow->total_production;
}

float EconomicFlowSystem::getTotalConsumption(const std::string& system_id) const {
    const auto* flow = getComponentFor(system_id);
    if (!flow) return 0.0f;

    return flow->total_consumption;
}

} // namespace systems
} // namespace atlas
