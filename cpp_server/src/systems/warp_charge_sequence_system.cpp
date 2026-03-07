#include "systems/warp_charge_sequence_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

WarpChargeSequenceSystem::WarpChargeSequenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void WarpChargeSequenceSystem::updateComponent(ecs::Entity& entity,
    components::WarpChargeState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Tick cooldown
    if (comp.cooldown_remaining > 0.0f) {
        comp.cooldown_remaining = std::max(0.0f, comp.cooldown_remaining - delta_time);
    }

    // Tick charge if charging and aligned
    if (comp.charging && comp.aligned) {
        float charge_time = comp.base_charge_time * comp.mass_factor;
        if (charge_time > 0.0f) {
            comp.charge_progress += delta_time / charge_time;
            comp.charge_progress = std::min(1.0f, comp.charge_progress);
        }
    }
}

bool WarpChargeSequenceSystem::initialize(const std::string& entity_id,
    float base_charge_time, float mass_factor) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (base_charge_time <= 0.0f || mass_factor <= 0.0f) return false;

    auto comp = std::make_unique<components::WarpChargeState>();
    comp->base_charge_time = base_charge_time;
    comp->mass_factor = mass_factor;
    entity->addComponent(std::move(comp));
    return true;
}

bool WarpChargeSequenceSystem::initiateWarp(const std::string& entity_id,
    const std::string& destination_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->charging) return false;  // already charging
    if (comp->cooldown_remaining > 0.0f) return false;  // still on cooldown
    if (destination_id.empty()) return false;

    comp->destination_id = destination_id;
    comp->charging = true;
    comp->charge_progress = 0.0f;
    return true;
}

bool WarpChargeSequenceSystem::disruptWarp(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->charging) return false;

    comp->charging = false;
    comp->charge_progress = 0.0f;
    comp->destination_id.clear();
    comp->aligned = false;
    comp->total_disruptions++;
    return true;
}

bool WarpChargeSequenceSystem::completeWarp(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->charging) return false;
    if (comp->charge_progress < 1.0f) return false;  // not fully charged

    comp->charging = false;
    comp->charge_progress = 0.0f;
    comp->cooldown_remaining = comp->cooldown_duration;
    comp->aligned = false;
    comp->destination_id.clear();
    comp->total_warps_completed++;
    return true;
}

bool WarpChargeSequenceSystem::setAligned(const std::string& entity_id, bool aligned) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->aligned = aligned;
    return true;
}

float WarpChargeSequenceSystem::getChargeProgress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->charge_progress : 0.0f;
}

float WarpChargeSequenceSystem::getCooldownRemaining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cooldown_remaining : 0.0f;
}

std::string WarpChargeSequenceSystem::getDestination(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->destination_id : "";
}

int WarpChargeSequenceSystem::getTotalWarpsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_warps_completed : 0;
}

int WarpChargeSequenceSystem::getTotalDisruptions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_disruptions : 0;
}

bool WarpChargeSequenceSystem::isCharging(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->charging : false;
}

bool WarpChargeSequenceSystem::isOnCooldown(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->cooldown_remaining > 0.0f) : false;
}

} // namespace systems
} // namespace atlas
