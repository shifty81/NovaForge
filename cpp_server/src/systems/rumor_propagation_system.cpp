#include "systems/rumor_propagation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/narrative_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::RumorPropagation::Rumor* findRumor(
    components::RumorPropagation* rp, const std::string& rumor_id) {
    for (auto& r : rp->rumors) {
        if (r.rumor_id == rumor_id) return &r;
    }
    return nullptr;
}
} // anonymous namespace

RumorPropagationSystem::RumorPropagationSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void RumorPropagationSystem::updateComponent(ecs::Entity& entity, components::RumorPropagation& comp, float delta_time) {
    if (!comp.active) return;

    for (auto& rumor : comp.rumors) {
        if (rumor.expired || rumor.confirmed) continue;

        rumor.age += delta_time;

        // Decay accuracy over time (confirmed rumors don't decay)
        rumor.accuracy = std::max(0.0f, rumor.accuracy - rumor.decay_rate * delta_time);

        // Check for expiry
        if (rumor.accuracy < comp.expiry_threshold) {
            rumor.expired = true;
            comp.total_expired++;
        }
    }
}

bool RumorPropagationSystem::initializeNetwork(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RumorPropagation>();
    entity->addComponent(std::move(comp));
    return true;
}

bool RumorPropagationSystem::createRumor(const std::string& entity_id,
    const std::string& rumor_id, const std::string& category, float accuracy) {
    auto* rp = getComponentFor(entity_id);
    if (!rp) return false;
    if (static_cast<int>(rp->rumors.size()) >= rp->max_rumors) return false;
    if (findRumor(rp, rumor_id)) return false; // duplicate

    components::RumorPropagation::Rumor rumor;
    rumor.rumor_id = rumor_id;
    rumor.category = category;
    rumor.accuracy = std::max(0.0f, std::min(1.0f, accuracy));
    rp->rumors.push_back(rumor);
    return true;
}

bool RumorPropagationSystem::spreadRumor(const std::string& entity_id,
    const std::string& rumor_id, const std::string& target_system) {
    auto* rp = getComponentFor(entity_id);
    if (!rp) return false;
    auto* rumor = findRumor(rp, rumor_id);
    if (!rumor || rumor->expired) return false;

    // Check if already spread to this system
    for (const auto& sys : rumor->reached_systems) {
        if (sys == target_system) return false;
    }

    rumor->reached_systems.push_back(target_system);
    rumor->spread_count++;
    // Each spread reduces accuracy slightly
    rumor->accuracy = std::max(0.0f, rumor->accuracy * 0.9f);
    return true;
}

bool RumorPropagationSystem::confirmRumor(const std::string& entity_id,
    const std::string& rumor_id) {
    auto* rp = getComponentFor(entity_id);
    if (!rp) return false;
    auto* rumor = findRumor(rp, rumor_id);
    if (!rumor || rumor->expired || rumor->confirmed) return false;

    rumor->confirmed = true;
    rumor->accuracy = 1.0f;
    rp->total_confirmed++;
    return true;
}

float RumorPropagationSystem::getRumorAccuracy(const std::string& entity_id,
    const std::string& rumor_id) const {
    const auto* rp = getComponentFor(entity_id);
    if (!rp) return 0.0f;
    for (const auto& r : rp->rumors) {
        if (r.rumor_id == rumor_id) return r.accuracy;
    }
    return 0.0f;
}

int RumorPropagationSystem::getRumorCount(const std::string& entity_id) const {
    const auto* rp = getComponentFor(entity_id);
    if (!rp) return 0;
    return static_cast<int>(rp->rumors.size());
}

int RumorPropagationSystem::getConfirmedCount(const std::string& entity_id) const {
    const auto* rp = getComponentFor(entity_id);
    if (!rp) return 0;
    return rp->total_confirmed;
}

int RumorPropagationSystem::getExpiredCount(const std::string& entity_id) const {
    const auto* rp = getComponentFor(entity_id);
    if (!rp) return 0;
    return rp->total_expired;
}

int RumorPropagationSystem::getSpreadCount(const std::string& entity_id,
    const std::string& rumor_id) const {
    const auto* rp = getComponentFor(entity_id);
    if (!rp) return 0;
    for (const auto& r : rp->rumors) {
        if (r.rumor_id == rumor_id) return r.spread_count;
    }
    return 0;
}

bool RumorPropagationSystem::isRumorActive(const std::string& entity_id,
    const std::string& rumor_id) const {
    const auto* rp = getComponentFor(entity_id);
    if (!rp) return false;
    for (const auto& r : rp->rumors) {
        if (r.rumor_id == rumor_id) return !r.expired;
    }
    return false;
}

} // namespace systems
} // namespace atlas
