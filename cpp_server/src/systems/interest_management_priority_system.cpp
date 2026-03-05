#include "systems/interest_management_priority_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
void applyTierSettings(components::InterestPriority* ip, int tier) {
    switch (tier) {
        case 0: ip->update_interval = 0.05f; ip->bandwidth_weight = 1.0f; break;
        case 1: ip->update_interval = 0.1f;  ip->bandwidth_weight = 0.75f; break;
        case 2: ip->update_interval = 0.25f; ip->bandwidth_weight = 0.5f; break;
        case 3: ip->update_interval = 0.5f;  ip->bandwidth_weight = 0.25f; break;
    }
}
} // anonymous namespace

InterestManagementPrioritySystem::InterestManagementPrioritySystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void InterestManagementPrioritySystem::updateComponent(ecs::Entity& /*entity*/, components::InterestPriority& ip, float delta_time) {
    if (!ip.active) return;

    ip.time_since_update += delta_time;
    if (ip.time_since_update >= ip.update_interval) {
        ip.needs_update = true;
        ip.time_since_update = 0.0f;
        ip.total_updates++;
    }
}

bool InterestManagementPrioritySystem::createPriority(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::InterestPriority>();
    entity->addComponent(std::move(comp));
    return true;
}

bool InterestManagementPrioritySystem::setClientId(const std::string& entity_id, int client_id) {
    auto* ip = getComponentFor(entity_id);
    if (!ip) return false;
    ip->client_id = client_id;
    return true;
}

bool InterestManagementPrioritySystem::setPriorityTier(const std::string& entity_id, int tier) {
    auto* ip = getComponentFor(entity_id);
    if (!ip) return false;
    ip->priority_tier = std::max(0, std::min(tier, 3));
    applyTierSettings(ip, ip->priority_tier);
    return true;
}

bool InterestManagementPrioritySystem::setDistance(const std::string& entity_id, float distance) {
    auto* ip = getComponentFor(entity_id);
    if (!ip) return false;
    ip->distance = distance;

    // Auto-compute tier based on distance
    int tier;
    if (distance < 1000.0f) tier = 0;
    else if (distance < 5000.0f) tier = 1;
    else if (distance < 20000.0f) tier = 2;
    else tier = 3;

    ip->priority_tier = tier;
    applyTierSettings(ip, tier);
    return true;
}

bool InterestManagementPrioritySystem::needsUpdate(const std::string& entity_id) const {
    const auto* ip = getComponentFor(entity_id);
    if (!ip) return false;
    return ip->needs_update;
}

int InterestManagementPrioritySystem::getPriorityTier(const std::string& entity_id) const {
    const auto* ip = getComponentFor(entity_id);
    if (!ip) return -1;
    return ip->priority_tier;
}

float InterestManagementPrioritySystem::getBandwidthWeight(const std::string& entity_id) const {
    const auto* ip = getComponentFor(entity_id);
    if (!ip) return 0.0f;
    return ip->bandwidth_weight;
}

int InterestManagementPrioritySystem::getTotalUpdates(const std::string& entity_id) const {
    const auto* ip = getComponentFor(entity_id);
    if (!ip) return 0;
    return ip->total_updates;
}

float InterestManagementPrioritySystem::getEstimatedBandwidth(const std::string& entity_id) const {
    const auto* ip = getComponentFor(entity_id);
    if (!ip) return 0.0f;
    return ip->bandwidth_weight * 100.0f;
}

} // namespace systems
} // namespace atlas
