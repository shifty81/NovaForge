#include "systems/resource_production_chain_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ResourceProductionChainSystem::ResourceProductionChainSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void ResourceProductionChainSystem::updateComponent(ecs::Entity& /*entity*/, components::ResourceProductionChain& comp, float delta_time) {
    if (!comp.is_active) return;

    comp.uptime += delta_time;

    // Calculate bottleneck factors (downstream capacity limits upstream)
    for (size_t i = 0; i < comp.stages.size(); i++) {
        if (i + 1 < comp.stages.size()) {
            // Downstream stage limits upstream: if downstream is inefficient,
            // upstream gets bottlenecked
            float downstream_capacity = comp.stages[i + 1].efficiency * comp.stages[i + 1].conversion_rate;
            float upstream_output = comp.stages[i].efficiency * comp.stages[i].conversion_rate;
            if (upstream_output > 0.0f && downstream_capacity < upstream_output) {
                comp.stages[i].bottleneck_factor = std::clamp(downstream_capacity / upstream_output, 0.0f, 1.0f);
            } else {
                comp.stages[i].bottleneck_factor = 1.0f;
            }
        } else {
            comp.stages[i].bottleneck_factor = 1.0f; // last stage has no downstream bottleneck
        }
    }

    // Calculate throughput for each stage
    float input_flow = 1.0f; // base input rate
    for (auto& stage : comp.stages) {
        stage.throughput = input_flow * stage.conversion_rate * stage.efficiency * stage.bottleneck_factor;
        input_flow = stage.throughput; // output of this stage becomes input for next
    }

    // Calculate overall efficiency (product of all stage efficiencies)
    float overall = 1.0f;
    for (const auto& stage : comp.stages) {
        overall *= stage.efficiency;
    }
    comp.overall_efficiency = overall;

    // Total output is the throughput of the last stage
    if (!comp.stages.empty()) {
        comp.total_output = comp.stages.back().throughput;
    } else {
        comp.total_output = 0.0f;
    }
}

bool ResourceProductionChainSystem::createChain(const std::string& entity_id, const std::string& chain_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* chain = entity->getComponent<components::ResourceProductionChain>();
    if (chain) return false; // already has a chain

    auto chain_comp = std::make_unique<components::ResourceProductionChain>();
    chain_comp->chain_id = chain_id;
    chain_comp->is_active = true;
    entity->addComponent(std::move(chain_comp));
    return true;
}

bool ResourceProductionChainSystem::addStage(const std::string& entity_id, const std::string& stage_name,
                                              const std::string& input_resource, const std::string& output_resource,
                                              float conversion_rate) {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return false;

    // Check for duplicate stage name
    for (const auto& s : chain->stages) {
        if (s.stage_name == stage_name) return false;
    }

    components::ResourceProductionChain::ChainStage stage;
    stage.stage_name = stage_name;
    stage.input_resource = input_resource;
    stage.output_resource = output_resource;
    stage.conversion_rate = conversion_rate;
    stage.efficiency = 1.0f;
    stage.bottleneck_factor = 1.0f;
    stage.throughput = 0.0f;
    chain->stages.push_back(stage);
    return true;
}

bool ResourceProductionChainSystem::removeStage(const std::string& entity_id, const std::string& stage_name) {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return false;

    auto it = std::remove_if(chain->stages.begin(), chain->stages.end(),
        [&stage_name](const components::ResourceProductionChain::ChainStage& s) {
            return s.stage_name == stage_name;
        });

    if (it == chain->stages.end()) return false;
    chain->stages.erase(it, chain->stages.end());
    return true;
}

bool ResourceProductionChainSystem::setStageEfficiency(const std::string& entity_id, const std::string& stage_name, float efficiency) {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return false;

    for (auto& stage : chain->stages) {
        if (stage.stage_name == stage_name) {
            stage.efficiency = std::clamp(efficiency, 0.0f, 1.0f);
            return true;
        }
    }
    return false;
}

bool ResourceProductionChainSystem::setChainActive(const std::string& entity_id, bool active) {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return false;

    chain->is_active = active;
    return true;
}

float ResourceProductionChainSystem::getOverallEfficiency(const std::string& entity_id) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return 0.0f;

    return chain->overall_efficiency;
}

float ResourceProductionChainSystem::getTotalOutput(const std::string& entity_id) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return 0.0f;

    return chain->total_output;
}

float ResourceProductionChainSystem::getStageThroughput(const std::string& entity_id, const std::string& stage_name) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return 0.0f;

    for (const auto& stage : chain->stages) {
        if (stage.stage_name == stage_name) return stage.throughput;
    }
    return 0.0f;
}

float ResourceProductionChainSystem::getBottleneckFactor(const std::string& entity_id, const std::string& stage_name) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return 1.0f;

    for (const auto& stage : chain->stages) {
        if (stage.stage_name == stage_name) return stage.bottleneck_factor;
    }
    return 1.0f;
}

int ResourceProductionChainSystem::getStageCount(const std::string& entity_id) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return 0;

    return static_cast<int>(chain->stages.size());
}

bool ResourceProductionChainSystem::isChainActive(const std::string& entity_id) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return false;

    return chain->is_active;
}

float ResourceProductionChainSystem::getUptime(const std::string& entity_id) const {
    auto* chain = getComponentFor(entity_id);
    if (!chain) return 0.0f;

    return chain->uptime;
}

} // namespace systems
} // namespace atlas
