#include "systems/stasis_web_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

StasisWebSystem::StasisWebSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void StasisWebSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::StasisWebState& comp,
        float delta_time) {
    comp.elapsed += delta_time;

    bool any_active = false;
    for (auto& web : comp.webs) {
        web.cycle_elapsed += delta_time;
        if (web.cycle_elapsed >= web.cycle_time) {
            web.cycle_elapsed -= web.cycle_time;
        }
        any_active = true;
    }
    comp.is_webbed = any_active;
    recomputeVelocity(comp);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool StasisWebSystem::initialize(const std::string& entity_id,
                                  float base_velocity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (base_velocity <= 0.0f) return false;
    auto comp = std::make_unique<components::StasisWebState>();
    comp->base_velocity      = base_velocity;
    comp->effective_velocity = base_velocity;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Web management
// ---------------------------------------------------------------------------

bool StasisWebSystem::applyWeb(const std::string& entity_id,
                                const std::string& web_id,
                                const std::string& source_id,
                                float strength,
                                float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (web_id.empty() || source_id.empty()) return false;
    if (strength <= 0.0f || strength >= 1.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->webs.size()) >= comp->max_webs) return false;

    for (const auto& w : comp->webs) {
        if (w.web_id == web_id) return false;
    }

    components::StasisWebState::Web web;
    web.web_id    = web_id;
    web.source_id = source_id;
    web.strength  = strength;
    web.cycle_time = cycle_time;
    web.active    = true;
    comp->webs.push_back(web);
    comp->total_webs_applied++;
    comp->is_webbed = true;
    recomputeVelocity(*comp);
    return true;
}

bool StasisWebSystem::removeWeb(const std::string& entity_id,
                                 const std::string& web_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->webs.begin(), comp->webs.end(),
        [&](const components::StasisWebState::Web& w) {
            return w.web_id == web_id;
        });
    if (it == comp->webs.end()) return false;
    comp->webs.erase(it);
    comp->is_webbed = !comp->webs.empty();
    recomputeVelocity(*comp);
    return true;
}

bool StasisWebSystem::clearWebs(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->webs.clear();
    comp->is_webbed = false;
    recomputeVelocity(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool StasisWebSystem::setBaseVelocity(const std::string& entity_id,
                                       float velocity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (velocity <= 0.0f) return false;
    comp->base_velocity = velocity;
    recomputeVelocity(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float StasisWebSystem::getEffectiveVelocity(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_velocity : 0.0f;
}

float StasisWebSystem::getBaseVelocity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->base_velocity : 0.0f;
}

int StasisWebSystem::getWebCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->webs.size()) : 0;
}

int StasisWebSystem::getActiveWebCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& w : comp->webs) {
        if (w.active) count++;
    }
    return count;
}

bool StasisWebSystem::isWebbed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->is_webbed : false;
}

int StasisWebSystem::getTotalWebsApplied(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_webs_applied : 0;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void StasisWebSystem::recomputeVelocity(components::StasisWebState& comp) {
    float factor = 1.0f;
    for (const auto& w : comp.webs) {
        factor *= (1.0f - w.strength);
    }
    comp.effective_velocity = comp.base_velocity * factor;
}

} // namespace systems
} // namespace atlas
