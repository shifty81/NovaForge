#include "systems/jump_clone_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

JumpCloneSystem::JumpCloneSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — count down jump cooldown
// ---------------------------------------------------------------------------

void JumpCloneSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::JumpCloneState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.cooldown_remaining > 0.0f) {
        comp.cooldown_remaining -= delta_time;
        if (comp.cooldown_remaining < 0.0f) {
            comp.cooldown_remaining = 0.0f;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool JumpCloneSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::JumpCloneState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Clone management
// ---------------------------------------------------------------------------

bool JumpCloneSystem::installClone(const std::string& entity_id,
                                    const std::string& clone_id,
                                    const std::string& station_id,
                                    const std::string& station_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (clone_id.empty()) return false;
    if (station_id.empty()) return false;
    if (station_name.empty()) return false;

    // Duplicate check
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->clones.size()) >= comp->max_clones) return false;

    components::JumpCloneState::JumpClone c;
    c.clone_id     = clone_id;
    c.station_id   = station_id;
    c.station_name = station_name;
    comp->clones.push_back(c);
    comp->total_clones_installed++;
    return true;
}

bool JumpCloneSystem::destroyClone(const std::string& entity_id,
                                    const std::string& clone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->clones.begin(), comp->clones.end(),
        [&](const components::JumpCloneState::JumpClone& c) {
            return c.clone_id == clone_id;
        });
    if (it == comp->clones.end()) return false;

    // Cannot destroy the active clone
    if (comp->active_clone_id == clone_id) return false;

    comp->clones.erase(it);
    comp->total_clones_destroyed++;
    return true;
}

bool JumpCloneSystem::clearClones(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->clones.clear();
    comp->active_clone_id.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Implant management
// ---------------------------------------------------------------------------

bool JumpCloneSystem::addImplant(const std::string& entity_id,
                                  const std::string& clone_id,
                                  const std::string& implant_id,
                                  const std::string& implant_name,
                                  int slot) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (implant_id.empty()) return false;
    if (implant_name.empty()) return false;
    if (slot < 1 || slot > 10) return false;

    for (auto& c : comp->clones) {
        if (c.clone_id == clone_id) {
            // Duplicate implant check
            for (const auto& imp : c.implants) {
                if (imp.implant_id == implant_id) return false;
            }
            // Slot occupancy check
            for (const auto& imp : c.implants) {
                if (imp.slot == slot) return false;
            }
            components::JumpCloneState::Implant imp;
            imp.implant_id = implant_id;
            imp.name       = implant_name;
            imp.slot       = slot;
            c.implants.push_back(imp);
            return true;
        }
    }
    return false; // clone not found
}

bool JumpCloneSystem::removeImplant(const std::string& entity_id,
                                     const std::string& clone_id,
                                     const std::string& implant_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& c : comp->clones) {
        if (c.clone_id == clone_id) {
            auto it = std::find_if(c.implants.begin(), c.implants.end(),
                [&](const components::JumpCloneState::Implant& imp) {
                    return imp.implant_id == implant_id;
                });
            if (it == c.implants.end()) return false;
            c.implants.erase(it);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Jump operations
// ---------------------------------------------------------------------------

bool JumpCloneSystem::jumpToClone(const std::string& entity_id,
                                   const std::string& clone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (clone_id.empty()) return false;
    if (comp->cooldown_remaining > 0.0f) return false;

    // Cannot jump to the same clone
    if (comp->active_clone_id == clone_id) return false;

    // Verify clone exists
    bool found = false;
    std::string target_station;
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id) {
            found = true;
            target_station = c.station_id;
            break;
        }
    }
    if (!found) return false;

    comp->active_clone_id   = clone_id;
    comp->active_station_id = target_station;
    comp->cooldown_remaining = comp->cooldown_duration;
    comp->total_jumps++;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool JumpCloneSystem::setMaxClones(const std::string& entity_id,
                                    int max_clones) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_clones <= 0) return false;
    comp->max_clones = max_clones;
    return true;
}

bool JumpCloneSystem::setCooldownDuration(const std::string& entity_id,
                                           float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (duration < 0.0f) return false;
    comp->cooldown_duration = duration;
    return true;
}

bool JumpCloneSystem::setActiveStation(const std::string& entity_id,
                                        const std::string& station_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->active_station_id = station_id;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int JumpCloneSystem::getCloneCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->clones.size()) : 0;
}

bool JumpCloneSystem::hasClone(const std::string& entity_id,
                                const std::string& clone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id) return true;
    }
    return false;
}

std::string JumpCloneSystem::getCloneStation(
        const std::string& entity_id,
        const std::string& clone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id) return c.station_id;
    }
    return "";
}

std::string JumpCloneSystem::getCloneStationName(
        const std::string& entity_id,
        const std::string& clone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id) return c.station_name;
    }
    return "";
}

int JumpCloneSystem::getImplantCount(const std::string& entity_id,
                                      const std::string& clone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id)
            return static_cast<int>(c.implants.size());
    }
    return 0;
}

bool JumpCloneSystem::hasImplant(const std::string& entity_id,
                                  const std::string& clone_id,
                                  const std::string& implant_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& c : comp->clones) {
        if (c.clone_id == clone_id) {
            for (const auto& imp : c.implants) {
                if (imp.implant_id == implant_id) return true;
            }
            return false;
        }
    }
    return false;
}

float JumpCloneSystem::getCooldownRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cooldown_remaining : 0.0f;
}

bool JumpCloneSystem::isOnCooldown(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->cooldown_remaining > 0.0f;
}

std::string JumpCloneSystem::getActiveCloneId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_clone_id : "";
}

std::string JumpCloneSystem::getActiveStationId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_station_id : "";
}

int JumpCloneSystem::getTotalJumps(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jumps : 0;
}

int JumpCloneSystem::getTotalClonesInstalled(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_clones_installed : 0;
}

int JumpCloneSystem::getTotalClonesDestroyed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_clones_destroyed : 0;
}

} // namespace systems
} // namespace atlas
