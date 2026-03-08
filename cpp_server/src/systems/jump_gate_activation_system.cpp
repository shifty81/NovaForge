#include "systems/jump_gate_activation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

JumpGateActivationSystem::JumpGateActivationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void JumpGateActivationSystem::updateComponent(ecs::Entity& entity,
    components::JumpGateState& jgs, float delta_time) {
    if (!jgs.active) return;

    if (jgs.charging) {
        float charge_increment = delta_time / jgs.charge_time;
        jgs.current_charge = std::min(1.0f, jgs.current_charge + charge_increment);
        if (jgs.current_charge >= 1.0f) {
            jgs.charging = false;
            jgs.on_cooldown = true;
            jgs.current_cooldown = jgs.cooldown_time;
            jgs.current_charge = 0.0f;
            jgs.total_jumps++;
            if (jgs.current_queue > 0) jgs.current_queue--;
        }
    } else if (jgs.on_cooldown) {
        jgs.current_cooldown -= delta_time;
        if (jgs.current_cooldown <= 0.0f) {
            jgs.current_cooldown = 0.0f;
            jgs.on_cooldown = false;
        }
    }

    jgs.elapsed += delta_time;
}

bool JumpGateActivationSystem::initialize(const std::string& entity_id,
    const std::string& gate_id, const std::string& dest_system,
    const std::string& dest_gate) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (gate_id.empty() || dest_system.empty() || dest_gate.empty()) return false;
    auto comp = std::make_unique<components::JumpGateState>();
    comp->gate_id = gate_id;
    comp->destination_system = dest_system;
    comp->destination_gate_id = dest_gate;
    entity->addComponent(std::move(comp));
    return true;
}

bool JumpGateActivationSystem::startCharge(const std::string& entity_id) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs || !jgs->isReady()) return false;
    jgs->charging = true;
    jgs->current_charge = 0.0f;
    return true;
}

bool JumpGateActivationSystem::cancelCharge(const std::string& entity_id) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs || !jgs->charging) return false;
    jgs->charging = false;
    jgs->current_charge = 0.0f;
    return true;
}

bool JumpGateActivationSystem::queueShip(const std::string& entity_id) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs) return false;
    if (jgs->current_queue >= jgs->max_queue) return false;
    jgs->current_queue++;
    return true;
}

bool JumpGateActivationSystem::dequeueShip(const std::string& entity_id) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs || jgs->current_queue <= 0) return false;
    jgs->current_queue--;
    return true;
}

bool JumpGateActivationSystem::setChargeTime(const std::string& entity_id, float time) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs) return false;
    jgs->charge_time = std::max(1.0f, std::min(60.0f, time));
    return true;
}

bool JumpGateActivationSystem::setCooldownTime(const std::string& entity_id, float time) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs) return false;
    jgs->cooldown_time = std::max(0.0f, std::min(300.0f, time));
    return true;
}

bool JumpGateActivationSystem::setFuelCost(const std::string& entity_id, float cost) {
    auto* jgs = getComponentFor(entity_id);
    if (!jgs) return false;
    jgs->fuel_cost = std::max(0.0f, cost);
    return true;
}

float JumpGateActivationSystem::getChargeProgress(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->current_charge : 0.0f;
}

float JumpGateActivationSystem::getRemainingCooldown(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->current_cooldown : 0.0f;
}

float JumpGateActivationSystem::getFuelCost(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->fuel_cost : 0.0f;
}

bool JumpGateActivationSystem::isReady(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->isReady() : false;
}

bool JumpGateActivationSystem::isCharging(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->charging : false;
}

bool JumpGateActivationSystem::isOnCooldown(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->on_cooldown : false;
}

int JumpGateActivationSystem::getTotalJumps(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->total_jumps : 0;
}

int JumpGateActivationSystem::getCurrentQueue(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->current_queue : 0;
}

std::string JumpGateActivationSystem::getDestinationSystem(const std::string& entity_id) const {
    auto* jgs = getComponentFor(entity_id);
    return jgs ? jgs->destination_system : "";
}

} // namespace systems
} // namespace atlas
