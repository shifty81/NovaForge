#include "systems/command_burst_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CommandBurstSystem::CommandBurstSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void CommandBurstSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CommandBurstState& comp,
        float delta_time) {
    comp.elapsed += delta_time;

    for (auto& burst : comp.bursts) {
        if (burst.active) {
            burst.cycle_elapsed += delta_time;
            if (burst.cycle_elapsed >= burst.cycle_time) {
                burst.cycle_elapsed -= burst.cycle_time;
                burst.active = false;
                comp.total_cycles++;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool CommandBurstSystem::initialize(const std::string& entity_id,
                                     const std::string& commander_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity || commander_id.empty()) return false;
    auto comp = std::make_unique<components::CommandBurstState>();
    comp->commander_id = commander_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Burst management
// ---------------------------------------------------------------------------

bool CommandBurstSystem::addBurst(
        const std::string& entity_id,
        const std::string& burst_id,
        components::CommandBurstState::BurstType type,
        float strength,
        float radius,
        float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (burst_id.empty()) return false;
    if (strength <= 0.0f || strength > 1.0f) return false;
    if (radius <= 0.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->bursts.size()) >= comp->max_bursts) return false;

    for (const auto& b : comp->bursts) {
        if (b.burst_id == burst_id) return false;
    }

    components::CommandBurstState::Burst burst;
    burst.burst_id    = burst_id;
    burst.type        = type;
    burst.strength    = strength;
    burst.radius      = radius;
    burst.cycle_time  = cycle_time;
    comp->bursts.push_back(burst);
    return true;
}

bool CommandBurstSystem::removeBurst(const std::string& entity_id,
                                      const std::string& burst_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->bursts.begin(), comp->bursts.end(),
        [&](const components::CommandBurstState::Burst& b) {
            return b.burst_id == burst_id;
        });
    if (it == comp->bursts.end()) return false;
    comp->bursts.erase(it);
    return true;
}

bool CommandBurstSystem::activateBurst(const std::string& entity_id,
                                        const std::string& burst_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& b : comp->bursts) {
        if (b.burst_id == burst_id) {
            if (b.active) return false;
            b.active        = true;
            b.cycle_elapsed = 0.0f;
            b.activations++;
            comp->total_activations++;
            return true;
        }
    }
    return false;
}

bool CommandBurstSystem::deactivateBurst(const std::string& entity_id,
                                          const std::string& burst_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& b : comp->bursts) {
        if (b.burst_id == burst_id) {
            if (!b.active) return false;
            b.active = false;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int CommandBurstSystem::getBurstCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->bursts.size()) : 0;
}

int CommandBurstSystem::getActiveBurstCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& b : comp->bursts) {
        if (b.active) count++;
    }
    return count;
}

bool CommandBurstSystem::hasBurst(const std::string& entity_id,
                                   const std::string& burst_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& b : comp->bursts) {
        if (b.burst_id == burst_id) return true;
    }
    return false;
}

bool CommandBurstSystem::isBurstActive(const std::string& entity_id,
                                        const std::string& burst_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& b : comp->bursts) {
        if (b.burst_id == burst_id) return b.active;
    }
    return false;
}

bool CommandBurstSystem::isAnyActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& b : comp->bursts) {
        if (b.active) return true;
    }
    return false;
}

int CommandBurstSystem::getTotalActivations(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_activations : 0;
}

int CommandBurstSystem::getTotalCycles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cycles : 0;
}

std::string CommandBurstSystem::getCommanderId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->commander_id : std::string();
}

} // namespace systems
} // namespace atlas
