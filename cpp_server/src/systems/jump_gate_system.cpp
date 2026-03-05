#include "systems/jump_gate_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::JumpGate::Gate* findGate(components::JumpGate* jg, const std::string& gate_id) {
    for (auto& g : jg->gates) {
        if (g.gate_id == gate_id) return &g;
    }
    return nullptr;
}

const components::JumpGate::Gate* findGateConst(const components::JumpGate* jg, const std::string& gate_id) {
    for (const auto& g : jg->gates) {
        if (g.gate_id == gate_id) return &g;
    }
    return nullptr;
}
} // anonymous namespace

JumpGateSystem::JumpGateSystem(ecs::World* world) : SingleComponentSystem(world) {}

void JumpGateSystem::updateComponent(ecs::Entity& entity, components::JumpGate& jg, float delta_time) {
    if (!jg.active) return;

    for (auto& gate : jg.gates) {
        if (!gate.online) continue;

        // Advance cooldown
        if (gate.current_cooldown > 0.0f) {
            gate.current_cooldown -= delta_time;
            if (gate.current_cooldown < 0.0f) gate.current_cooldown = 0.0f;
        }

        // Advance activation progress
        if (gate.in_use && gate.activation_progress < 1.0f) {
            gate.activation_progress += delta_time / gate.activation_time;
            if (gate.activation_progress > 1.0f) gate.activation_progress = 1.0f;
        }
    }
}

bool JumpGateSystem::initializeGateNetwork(const std::string& entity_id,
    const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::JumpGate>();
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool JumpGateSystem::addGate(const std::string& entity_id, const std::string& gate_id,
    const std::string& dest_system, const std::string& dest_gate_id,
    float fuel_cost, float security_level) {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return false;
    if (static_cast<int>(jg->gates.size()) >= jg->max_gates) return false;
    if (findGate(jg, gate_id)) return false; // duplicate

    components::JumpGate::Gate gate;
    gate.gate_id = gate_id;
    gate.destination_system = dest_system;
    gate.destination_gate_id = dest_gate_id;
    gate.fuel_cost = fuel_cost;
    gate.security_level = security_level;
    jg->gates.push_back(gate);
    return true;
}

bool JumpGateSystem::removeGate(const std::string& entity_id, const std::string& gate_id) {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return false;

    auto it = std::remove_if(jg->gates.begin(), jg->gates.end(),
        [&](const components::JumpGate::Gate& g) { return g.gate_id == gate_id; });
    if (it == jg->gates.end()) return false;
    jg->gates.erase(it, jg->gates.end());
    return true;
}

bool JumpGateSystem::activateGate(const std::string& entity_id, const std::string& gate_id) {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return false;

    auto* gate = findGate(jg, gate_id);
    if (!gate || !gate->online) return false;
    if (gate->current_cooldown > 0.0f) return false; // still on cooldown
    if (gate->in_use) return false; // already activating

    gate->in_use = true;
    gate->activation_progress = 0.0f;
    jg->total_activations++;
    return true;
}

bool JumpGateSystem::completeJump(const std::string& entity_id, const std::string& gate_id) {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return false;

    auto* gate = findGate(jg, gate_id);
    if (!gate || !gate->in_use) return false;
    if (gate->activation_progress < 1.0f) return false; // not ready yet

    gate->in_use = false;
    gate->activation_progress = 0.0f;
    gate->current_cooldown = gate->cooldown_time;
    gate->total_jumps++;
    jg->total_jumps_processed++;
    return true;
}

bool JumpGateSystem::setGateOnline(const std::string& entity_id, const std::string& gate_id,
    bool online) {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return false;

    auto* gate = findGate(jg, gate_id);
    if (!gate) return false;
    gate->online = online;
    if (!online) {
        gate->in_use = false;
        gate->activation_progress = 0.0f;
    }
    return true;
}

float JumpGateSystem::getActivationProgress(const std::string& entity_id,
    const std::string& gate_id) const {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return 0.0f;
    auto* gate = findGateConst(jg, gate_id);
    return gate ? gate->activation_progress : 0.0f;
}

float JumpGateSystem::getCooldownRemaining(const std::string& entity_id,
    const std::string& gate_id) const {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return 0.0f;
    auto* gate = findGateConst(jg, gate_id);
    return gate ? gate->current_cooldown : 0.0f;
}

int JumpGateSystem::getGateCount(const std::string& entity_id) const {
    auto* jg = getComponentFor(entity_id);
    return jg ? static_cast<int>(jg->gates.size()) : 0;
}

int JumpGateSystem::getOnlineGateCount(const std::string& entity_id) const {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return 0;
    int count = 0;
    for (const auto& g : jg->gates) {
        if (g.online) count++;
    }
    return count;
}

int JumpGateSystem::getTotalJumps(const std::string& entity_id) const {
    auto* jg = getComponentFor(entity_id);
    return jg ? jg->total_jumps_processed : 0;
}

bool JumpGateSystem::isGateReady(const std::string& entity_id, const std::string& gate_id) const {
    auto* jg = getComponentFor(entity_id);
    if (!jg) return false;
    auto* gate = findGateConst(jg, gate_id);
    if (!gate) return false;
    return gate->online && !gate->in_use && gate->current_cooldown <= 0.0f;
}

} // namespace systems
} // namespace atlas
