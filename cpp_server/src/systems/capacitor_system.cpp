#include "systems/capacitor_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

CapacitorSystem::CapacitorSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CapacitorSystem::updateComponent(ecs::Entity& /*entity*/, components::Capacitor& cap, float delta_time) {
    // Recharge capacitor over time
    if (cap.capacitor < cap.capacitor_max) {
        float recharge = cap.recharge_rate * delta_time;
        cap.capacitor = std::min(cap.capacitor + recharge, cap.capacitor_max);
    }
}

bool CapacitorSystem::consumeCapacitor(const std::string& entity_id, float amount) {
    auto* cap = getComponentFor(entity_id);
    if (!cap) return false;
    
    if (cap->capacitor >= amount) {
        cap->capacitor -= amount;
        return true;
    }
    
    return false;  // Not enough capacitor
}

float CapacitorSystem::getCapacitorPercentage(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap || cap->capacitor_max <= 0.0f) return -1.0f;
    
    return cap->capacitor / cap->capacitor_max;
}

} // namespace systems
} // namespace atlas
