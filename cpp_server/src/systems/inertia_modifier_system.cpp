#include "systems/inertia_modifier_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

InertiaModifierSystem::InertiaModifierSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void InertiaModifierSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::InertiaModifierState& comp,
        float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Stacking-penalty recalculation
// ---------------------------------------------------------------------------

void InertiaModifierSystem::recalculate(components::InertiaModifierState& comp) {
    // Collect active reduction values, sorted strongest first
    std::vector<float> reductions;
    for (const auto& m : comp.modules) {
        if (m.is_active) {
            reductions.push_back(m.inertia_reduction);
        }
    }
    std::sort(reductions.begin(), reductions.end(), std::greater<float>());

    // EVE-style stacking penalty:  effectiveness_i = base * exp(-(i/2.67)^2)
    float inertia_multiplier = 1.0f;
    for (size_t i = 0; i < reductions.size(); ++i) {
        float penalty = std::exp(-static_cast<float>(i * i) / (2.67f * 2.67f));
        float effective_reduction = reductions[i] * penalty;
        inertia_multiplier *= (1.0f - effective_reduction);
    }

    comp.effective_inertia    = comp.base_inertia * inertia_multiplier;
    comp.effective_align_time = comp.base_align_time * inertia_multiplier;

    // Clamp to minimum 1% of base
    float min_inertia = comp.base_inertia * 0.01f;
    float min_align   = comp.base_align_time * 0.01f;
    if (comp.effective_inertia < min_inertia)
        comp.effective_inertia = min_inertia;
    if (comp.effective_align_time < min_align)
        comp.effective_align_time = min_align;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool InertiaModifierSystem::initialize(const std::string& entity_id,
                                        float base_inertia,
                                        float base_align_time) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (base_inertia <= 0.0f || base_align_time <= 0.0f) return false;

    auto comp = std::make_unique<components::InertiaModifierState>();
    comp->base_inertia        = base_inertia;
    comp->base_align_time     = base_align_time;
    comp->effective_inertia   = base_inertia;
    comp->effective_align_time = base_align_time;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Module management
// ---------------------------------------------------------------------------

bool InertiaModifierSystem::addModule(const std::string& entity_id,
                                       const std::string& module_id,
                                       const std::string& name,
                                       float inertia_reduction) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (module_id.empty() || name.empty()) return false;
    if (inertia_reduction <= 0.0f || inertia_reduction >= 1.0f) return false;
    if (static_cast<int>(comp->modules.size()) >= comp->max_modules) return false;

    // Duplicate prevention
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return false;
    }

    components::InertiaModifierState::InertiaModule mod;
    mod.module_id        = module_id;
    mod.name             = name;
    mod.inertia_reduction = inertia_reduction;
    mod.is_active        = true;
    comp->modules.push_back(mod);
    comp->total_modifications++;

    recalculate(*comp);
    return true;
}

bool InertiaModifierSystem::removeModule(const std::string& entity_id,
                                          const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->modules.begin(), comp->modules.end(),
        [&](const components::InertiaModifierState::InertiaModule& m) {
            return m.module_id == module_id;
        });
    if (it == comp->modules.end()) return false;
    comp->modules.erase(it);
    recalculate(*comp);
    return true;
}

bool InertiaModifierSystem::activateModule(const std::string& entity_id,
                                            const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (m.is_active) return false; // already active
            m.is_active = true;
            recalculate(*comp);
            return true;
        }
    }
    return false;
}

bool InertiaModifierSystem::deactivateModule(const std::string& entity_id,
                                              const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (!m.is_active) return false; // already inactive
            m.is_active = false;
            recalculate(*comp);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool InertiaModifierSystem::setBaseInertia(const std::string& entity_id,
                                            float inertia) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (inertia <= 0.0f) return false;
    comp->base_inertia = inertia;
    recalculate(*comp);
    return true;
}

bool InertiaModifierSystem::setBaseAlignTime(const std::string& entity_id,
                                              float align_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (align_time <= 0.0f) return false;
    comp->base_align_time = align_time;
    recalculate(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float InertiaModifierSystem::getEffectiveInertia(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_inertia : 0.0f;
}

float InertiaModifierSystem::getEffectiveAlignTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_align_time : 0.0f;
}

float InertiaModifierSystem::getBaseInertia(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->base_inertia : 0.0f;
}

float InertiaModifierSystem::getBaseAlignTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->base_align_time : 0.0f;
}

int InertiaModifierSystem::getModuleCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->modules.size()) : 0;
}

bool InertiaModifierSystem::isModuleActive(const std::string& entity_id,
                                            const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return m.is_active;
    }
    return false;
}

int InertiaModifierSystem::getTotalModifications(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_modifications : 0;
}

} // namespace systems
} // namespace atlas
