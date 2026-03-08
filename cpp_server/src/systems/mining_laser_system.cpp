#include "systems/mining_laser_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MiningLaserSystem::MiningLaserSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MiningLaserSystem::updateComponent(ecs::Entity& /*entity*/,
    components::MiningLaserState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (!comp.mining_active) return;
    if (comp.target_asteroid.empty()) return;
    if (comp.asteroid_remaining <= 0.0f) {
        comp.mining_active = false;
        return;
    }

    // Advance mining cycle
    if (comp.cycle_duration > 0.0f) {
        comp.cycle_progress += delta_time / comp.cycle_duration;
        while (comp.cycle_progress >= 1.0f) {
            comp.cycle_progress -= 1.0f;
            comp.total_cycles++;

            // Extract ore
            float yield = comp.mining_strength * 10.0f; // base 10 units per cycle
            comp.total_ore_mined += yield;
            comp.asteroid_remaining = std::max(0.0f,
                comp.asteroid_remaining - yield * 0.1f);

            if (comp.asteroid_remaining <= 0.0f) {
                comp.mining_active = false;
                comp.cycle_progress = 0.0f;
                break;
            }
        }
    }
}

bool MiningLaserSystem::initialize(const std::string& entity_id,
    const std::string& laser_type, float mining_strength) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (laser_type.empty()) return false;
    if (mining_strength <= 0.0f) return false;

    auto comp = std::make_unique<components::MiningLaserState>();
    comp->laser_type = laser_type;
    comp->mining_strength = mining_strength;
    entity->addComponent(std::move(comp));
    return true;
}

bool MiningLaserSystem::startMining(const std::string& entity_id,
    const std::string& target_asteroid) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_asteroid.empty()) return false;
    if (comp->mining_active) return false;
    if (comp->asteroid_remaining <= 0.0f) return false;

    comp->target_asteroid = target_asteroid;
    comp->mining_active = true;
    comp->cycle_progress = 0.0f;
    return true;
}

bool MiningLaserSystem::stopMining(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->mining_active) return false;

    comp->mining_active = false;
    if (comp->cycle_progress > 0.0f && comp->cycle_progress < 1.0f) {
        comp->failed_cycles++;
    }
    comp->cycle_progress = 0.0f;
    return true;
}

bool MiningLaserSystem::setRange(const std::string& entity_id,
    float range, float optimal_range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range <= 0.0f || optimal_range <= 0.0f) return false;
    if (optimal_range > range) return false;

    comp->range = range;
    comp->optimal_range = optimal_range;
    return true;
}

bool MiningLaserSystem::setCycleDuration(const std::string& entity_id, float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (duration <= 0.0f) return false;

    comp->cycle_duration = duration;
    return true;
}

bool MiningLaserSystem::addOreYield(const std::string& entity_id,
    const std::string& ore_type, float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ore_type.empty()) return false;
    if (amount <= 0.0f) return false;

    // Find or create ore entry
    for (auto& y : comp->cumulative_yield) {
        if (y.ore_type == ore_type) {
            y.amount += amount;
            return true;
        }
    }

    if (static_cast<int>(comp->cumulative_yield.size()) >= comp->max_ore_types)
        return false;

    components::MiningLaserState::OreYield entry;
    entry.ore_type = ore_type;
    entry.amount = amount;
    comp->cumulative_yield.push_back(entry);
    return true;
}

bool MiningLaserSystem::isMiningActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->mining_active : false;
}

float MiningLaserSystem::getCycleProgress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cycle_progress : 0.0f;
}

float MiningLaserSystem::getTotalOreMined(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ore_mined : 0.0f;
}

int MiningLaserSystem::getTotalCycles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cycles : 0;
}

int MiningLaserSystem::getFailedCycles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->failed_cycles : 0;
}

float MiningLaserSystem::getAsteroidRemaining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->asteroid_remaining : 0.0f;
}

std::string MiningLaserSystem::getTargetAsteroid(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->target_asteroid : "";
}

float MiningLaserSystem::getMiningStrength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->mining_strength : 0.0f;
}

int MiningLaserSystem::getOreTypeCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->cumulative_yield.size()) : 0;
}

float MiningLaserSystem::getOreYield(const std::string& entity_id,
    const std::string& ore_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& y : comp->cumulative_yield) {
        if (y.ore_type == ore_type) return y.amount;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
