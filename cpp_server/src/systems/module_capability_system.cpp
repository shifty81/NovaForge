#include "systems/module_capability_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ModuleCapabilitySystem::ModuleCapabilitySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void ModuleCapabilitySystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::ModuleCapabilityState& comp,
                                              float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool ModuleCapabilitySystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ModuleCapabilityState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Module management
// ---------------------------------------------------------------------------

bool ModuleCapabilitySystem::add_module(const std::string& entity_id,
                                         const std::string& module_id,
                                         const std::string& module_type,
                                         int size) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (module_id.empty()) return false;
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }

    components::ModuleCapabilityState::InstalledModule mod;
    mod.module_id   = module_id;
    mod.module_type = module_type;
    mod.size        = size;
    comp->modules.push_back(mod);
    return true;
}

bool ModuleCapabilitySystem::remove_module(const std::string& entity_id,
                                            const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
                           [&](const auto& m) { return m.module_id == module_id; });
    if (it == comp->modules.end()) return false;
    comp->modules.erase(it);
    return true;
}

bool ModuleCapabilitySystem::clear_modules(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->modules.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Capability management
// ---------------------------------------------------------------------------

bool ModuleCapabilitySystem::add_capability(const std::string& entity_id,
                                             const std::string& module_id,
                                             const std::string& capability_id,
                                             const std::string& capability_type,
                                             float strength) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            // Duplicate check
            for (const auto& c : m.capabilities) {
                if (c.capability_id == capability_id) return false;
            }
            components::ModuleCapabilityState::Capability cap;
            cap.capability_id   = capability_id;
            cap.capability_type = capability_type;
            cap.strength        = strength;
            m.capabilities.push_back(cap);
            comp->total_capabilities_registered++;
            return true;
        }
    }
    return false;
}

bool ModuleCapabilitySystem::remove_capability(const std::string& entity_id,
                                                const std::string& module_id,
                                                const std::string& capability_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            auto it = std::find_if(m.capabilities.begin(), m.capabilities.end(),
                                   [&](const auto& c) { return c.capability_id == capability_id; });
            if (it == m.capabilities.end()) return false;
            m.capabilities.erase(it);
            return true;
        }
    }
    return false;
}

bool ModuleCapabilitySystem::set_capability_enabled(const std::string& entity_id,
                                                     const std::string& module_id,
                                                     const std::string& capability_id,
                                                     bool enabled) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            for (auto& c : m.capabilities) {
                if (c.capability_id == capability_id) {
                    c.enabled = enabled;
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool ModuleCapabilitySystem::has_capability_type(
        const std::string& entity_id,
        const std::string& capability_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (const auto& m : comp->modules) {
        for (const auto& c : m.capabilities) {
            if (c.capability_type == capability_type && c.enabled) return true;
        }
    }
    return false;
}

float ModuleCapabilitySystem::get_total_capability_strength(
        const std::string& entity_id,
        const std::string& capability_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;

    float total = 0.0f;
    for (const auto& m : comp->modules) {
        for (const auto& c : m.capabilities) {
            if (c.capability_type == capability_type && c.enabled) {
                total += c.strength;
            }
        }
    }
    return total;
}

int ModuleCapabilitySystem::get_module_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->modules.size());
}

int ModuleCapabilitySystem::get_capability_count(const std::string& entity_id,
                                                  const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;

    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) {
            return static_cast<int>(m.capabilities.size());
        }
    }
    return 0;
}

bool ModuleCapabilitySystem::is_capability_enabled(const std::string& entity_id,
                                                    const std::string& module_id,
                                                    const std::string& capability_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) {
            for (const auto& c : m.capabilities) {
                if (c.capability_id == capability_id) return c.enabled;
            }
            return false;
        }
    }
    return false;
}

int ModuleCapabilitySystem::get_total_capabilities_registered(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_capabilities_registered;
}

} // namespace systems
} // namespace atlas
