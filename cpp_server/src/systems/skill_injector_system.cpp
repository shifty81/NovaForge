#include "systems/skill_injector_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SkillInjectorSystem::SkillInjectorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void SkillInjectorSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SkillInjectorState& comp,
        float delta_time) {
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SkillInjectorSystem::initialize(const std::string& entity_id,
                                      int unallocated_sp,
                                      int total_sp) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (unallocated_sp < 0 || total_sp < 0) return false;
    auto comp = std::make_unique<components::SkillInjectorState>();
    comp->unallocated_sp = unallocated_sp;
    comp->total_sp       = total_sp;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Core mechanics
// ---------------------------------------------------------------------------

static constexpr int EXTRACTION_COST = 500000;  // SP needed to fill one vial

bool SkillInjectorSystem::extractSP(const std::string& entity_id,
                                     const std::string& injector_id,
                                     const std::string& source_pilot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (injector_id.empty()) return false;
    if (comp->unallocated_sp < EXTRACTION_COST) return false;
    if (static_cast<int>(comp->injectors.size()) >= comp->max_injectors) return false;

    // Duplicate injector_id prevention
    for (const auto& inj : comp->injectors) {
        if (inj.injector_id == injector_id) return false;
    }

    components::SkillInjectorState::InjectorItem item;
    item.injector_id     = injector_id;
    item.skill_points    = EXTRACTION_COST;
    item.source_pilot_id = source_pilot_id;
    item.used            = false;

    comp->injectors.push_back(item);
    comp->unallocated_sp -= EXTRACTION_COST;
    comp->total_extracted += EXTRACTION_COST;
    return true;
}

bool SkillInjectorSystem::injectSP(const std::string& entity_id,
                                    const std::string& injector_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->injectors.begin(), comp->injectors.end(),
        [&](const components::SkillInjectorState::InjectorItem& inj) {
            return inj.injector_id == injector_id;
        });
    if (it == comp->injectors.end()) return false;
    if (it->used) return false;

    const int dose = computeDose(entity_id);
    it->used = true;
    comp->unallocated_sp += dose;
    comp->total_sp       += dose;
    comp->total_injected += dose;

    // Remove the used injector from inventory
    comp->injectors.erase(it);
    return true;
}

// ---------------------------------------------------------------------------
// Configuration helpers
// ---------------------------------------------------------------------------

bool SkillInjectorSystem::addUnallocatedSP(const std::string& entity_id,
                                             int amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0) return false;
    comp->unallocated_sp += amount;
    comp->total_sp       += amount;
    return true;
}

bool SkillInjectorSystem::setTotalSP(const std::string& entity_id,
                                      int total_sp) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (total_sp < 0) return false;
    comp->total_sp = total_sp;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int SkillInjectorSystem::getInjectorCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->injectors.size()) : 0;
}

int SkillInjectorSystem::getUnallocatedSP(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->unallocated_sp : 0;
}

int SkillInjectorSystem::getTotalSP(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_sp : 0;
}

int SkillInjectorSystem::getTotalExtracted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_extracted : 0;
}

int SkillInjectorSystem::getTotalInjected(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_injected : 0;
}

bool SkillInjectorSystem::hasInjector(const std::string& entity_id,
                                       const std::string& injector_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& inj : comp->injectors) {
        if (inj.injector_id == injector_id) return true;
    }
    return false;
}

int SkillInjectorSystem::computeDose(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    const int sp = comp->total_sp;
    if (sp <= 5'000'000)  return 500'000;
    if (sp <= 50'000'000) return 400'000;
    if (sp <= 80'000'000) return 300'000;
    return 200'000;
}

} // namespace systems
} // namespace atlas
