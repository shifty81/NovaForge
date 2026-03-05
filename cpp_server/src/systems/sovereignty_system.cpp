#include "systems/sovereignty_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

SovereigntySystem::SovereigntySystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void SovereigntySystem::updateComponent(ecs::Entity& entity, components::Sovereignty& sov, float delta_time) {
    float dt_hours = delta_time / 3600.0f;

    if (sov.is_contested) {
        // Decay control level slowly
        sov.control_level -= 0.01f * dt_hours;
        if (sov.control_level < 0.0f) sov.control_level = 0.0f;
    } else {
        // Increase control level toward 1.0
        sov.control_level += 0.01f * dt_hours;
        if (sov.control_level > 1.0f) sov.control_level = 1.0f;
    }

    // Decay indices slowly
    sov.military_index -= 0.005f * dt_hours;
    if (sov.military_index < 0.0f) sov.military_index = 0.0f;

    sov.industrial_index -= 0.005f * dt_hours;
    if (sov.industrial_index < 0.0f) sov.industrial_index = 0.0f;
}

bool SovereigntySystem::claimSovereignty(const std::string& system_entity_id,
                                         const std::string& owner_id,
                                         const std::string& system_name) {
    auto* entity = world_->getEntity(system_entity_id);
    if (!entity) {
        entity = world_->createEntity(system_entity_id);
        if (!entity) return false;
    }

    // Cannot claim already-owned systems
    auto* existing = entity->getComponent<components::Sovereignty>();
    if (existing) {
        if (!existing->owner_id.empty()) return false;
        // Reuse existing empty sovereignty component
        existing->system_id = system_entity_id;
        existing->owner_id = owner_id;
        existing->system_name = system_name;
        existing->control_level = 0.1f;
        return true;
    }

    auto sov = std::make_unique<components::Sovereignty>();
    sov->system_id = system_entity_id;
    sov->owner_id = owner_id;
    sov->system_name = system_name;
    sov->control_level = 0.1f;
    entity->addComponent(std::move(sov));

    return true;
}

bool SovereigntySystem::relinquishSovereignty(const std::string& system_entity_id,
                                              const std::string& requester_id) {
    auto* sov = getComponentFor(system_entity_id);
    if (!sov) return false;

    if (sov->owner_id != requester_id) return false;

    sov->owner_id.clear();
    sov->control_level = 0.0f;
    return true;
}

bool SovereigntySystem::contestSovereignty(const std::string& system_entity_id) {
    auto* sov = getComponentFor(system_entity_id);
    if (!sov) return false;

    sov->is_contested = true;
    return true;
}

bool SovereigntySystem::updateIndices(const std::string& system_entity_id,
                                      float military_delta,
                                      float industrial_delta) {
    auto* sov = getComponentFor(system_entity_id);
    if (!sov) return false;

    sov->military_index += military_delta;
    if (sov->military_index < 0.0f) sov->military_index = 0.0f;
    if (sov->military_index > 5.0f) sov->military_index = 5.0f;

    sov->industrial_index += industrial_delta;
    if (sov->industrial_index < 0.0f) sov->industrial_index = 0.0f;
    if (sov->industrial_index > 5.0f) sov->industrial_index = 5.0f;

    return true;
}

float SovereigntySystem::getControlLevel(const std::string& system_entity_id) {
    auto* sov = getComponentFor(system_entity_id);
    if (!sov) return 0.0f;

    return sov->control_level;
}

std::string SovereigntySystem::getOwner(const std::string& system_entity_id) {
    auto* sov = getComponentFor(system_entity_id);
    if (!sov) return "";

    return sov->owner_id;
}

bool SovereigntySystem::upgradeInfrastructure(const std::string& system_entity_id,
                                              const std::string& requester_id) {
    auto* sov = getComponentFor(system_entity_id);
    if (!sov) return false;

    if (sov->owner_id != requester_id) return false;
    if (sov->upgrade_level >= 5) return false;

    sov->upgrade_level++;
    return true;
}

} // namespace systems
} // namespace atlas
