#include "systems/asteroid_depletion_tracker_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

AsteroidDepletionTrackerSystem::AsteroidDepletionTrackerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AsteroidDepletionTrackerSystem::updateComponent(ecs::Entity& entity,
    components::AsteroidDepletionState& ads, float delta_time) {
    if (!ads.active) return;

    if (ads.depleted) {
        // Track time since depletion for respawn
        ads.time_since_depletion += delta_time;
        if (ads.time_since_depletion >= ads.respawn_delay) {
            // Respawn ore
            float respawn_amount = ads.respawn_rate * delta_time * ads.security_bonus;
            ads.remaining_ore = std::min(ads.total_ore_volume, ads.remaining_ore + respawn_amount);
            if (ads.remaining_ore >= ads.total_ore_volume) {
                ads.depleted = false;
                ads.time_since_depletion = 0.0f;
            }
        }
    } else {
        // Apply depletion from active miners
        if (ads.depletion_rate > 0.0f) {
            ads.remaining_ore -= ads.depletion_rate * delta_time;
            if (ads.remaining_ore <= 0.0f) {
                ads.remaining_ore = 0.0f;
                ads.depleted = true;
                ads.times_depleted++;
                ads.time_since_depletion = 0.0f;
            }
        }
    }

    ads.elapsed += delta_time;
}

bool AsteroidDepletionTrackerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AsteroidDepletionState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool AsteroidDepletionTrackerSystem::initialize(const std::string& entity_id, float total_volume) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (total_volume <= 0.0f) return false;
    auto comp = std::make_unique<components::AsteroidDepletionState>();
    comp->total_ore_volume = total_volume;
    comp->remaining_ore = total_volume;
    entity->addComponent(std::move(comp));
    return true;
}

bool AsteroidDepletionTrackerSystem::extractOre(const std::string& entity_id, float amount) {
    auto* ads = getComponentFor(entity_id);
    if (!ads || amount <= 0.0f || ads->depleted) return false;
    ads->remaining_ore = std::max(0.0f, ads->remaining_ore - amount);
    if (ads->remaining_ore <= 0.0f) {
        ads->depleted = true;
        ads->times_depleted++;
        ads->time_since_depletion = 0.0f;
    }
    return true;
}

bool AsteroidDepletionTrackerSystem::setActiveMiners(const std::string& entity_id, int count) {
    auto* ads = getComponentFor(entity_id);
    if (!ads) return false;
    ads->active_miners = std::max(0, count);
    return true;
}

bool AsteroidDepletionTrackerSystem::setSecurityBonus(const std::string& entity_id, float bonus) {
    auto* ads = getComponentFor(entity_id);
    if (!ads) return false;
    ads->security_bonus = std::max(0.5f, std::min(2.0f, bonus));
    return true;
}

bool AsteroidDepletionTrackerSystem::setRespawnRate(const std::string& entity_id, float rate) {
    auto* ads = getComponentFor(entity_id);
    if (!ads) return false;
    ads->respawn_rate = std::max(0.0f, rate);
    return true;
}

bool AsteroidDepletionTrackerSystem::setRespawnDelay(const std::string& entity_id, float delay) {
    auto* ads = getComponentFor(entity_id);
    if (!ads) return false;
    ads->respawn_delay = std::max(0.0f, delay);
    return true;
}

float AsteroidDepletionTrackerSystem::getRemainingOre(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->remaining_ore : 0.0f;
}

float AsteroidDepletionTrackerSystem::getTotalVolume(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->total_ore_volume : 0.0f;
}

float AsteroidDepletionTrackerSystem::getDepletionPercent(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->depletionPercent() : 0.0f;
}

bool AsteroidDepletionTrackerSystem::isDepleted(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->depleted : false;
}

int AsteroidDepletionTrackerSystem::getTimesDepleted(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->times_depleted : 0;
}

int AsteroidDepletionTrackerSystem::getActiveMiners(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->active_miners : 0;
}

float AsteroidDepletionTrackerSystem::getSecurityBonus(const std::string& entity_id) const {
    auto* ads = getComponentFor(entity_id);
    return ads ? ads->security_bonus : 0.0f;
}

} // namespace systems
} // namespace atlas
