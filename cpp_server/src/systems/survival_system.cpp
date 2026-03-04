#include "systems/survival_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

SurvivalSystem::SurvivalSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void SurvivalSystem::updateComponent(ecs::Entity& /*entity*/, components::SurvivalNeeds& needs, float delta_time) {
    needs.oxygen = std::max(0.0f, needs.oxygen - needs.oxygen_drain_rate * delta_time);
    needs.hunger = std::min(100.0f, needs.hunger + needs.hunger_rate * delta_time);
    needs.fatigue = std::min(100.0f, needs.fatigue + needs.fatigue_rate * delta_time);
}

float SurvivalSystem::refillOxygen(const std::string& entity_id, float amount) {
    auto* needs = getComponentFor(entity_id);
    if (!needs) return 0.0f;
    needs->oxygen = std::min(100.0f, needs->oxygen + amount);
    return needs->oxygen;
}

float SurvivalSystem::feed(const std::string& entity_id, float amount) {
    auto* needs = getComponentFor(entity_id);
    if (!needs) return 100.0f;
    needs->hunger = std::max(0.0f, needs->hunger - amount);
    return needs->hunger;
}

float SurvivalSystem::rest(const std::string& entity_id, float amount) {
    auto* needs = getComponentFor(entity_id);
    if (!needs) return 100.0f;
    needs->fatigue = std::max(0.0f, needs->fatigue - amount);
    return needs->fatigue;
}

bool SurvivalSystem::isAlive(const std::string& entity_id) const {
    const auto* needs = getComponentFor(entity_id);
    if (!needs) return false;
    return needs->isAlive();
}

std::tuple<float, float, float>
SurvivalSystem::getNeeds(const std::string& entity_id) const {
    const auto* needs = getComponentFor(entity_id);
    if (!needs) return {0.0f, 100.0f, 100.0f};
    return {needs->oxygen, needs->hunger, needs->fatigue};
}

} // namespace systems
} // namespace atlas
