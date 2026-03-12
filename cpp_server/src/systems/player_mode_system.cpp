#include "systems/player_mode_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlayerModeSystem::PlayerModeSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — advance transition progress
// ---------------------------------------------------------------------------

void PlayerModeSystem::updateComponent(ecs::Entity& /*entity*/,
                                        components::PlayerModeState& comp,
                                        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.in_transition && comp.transition_time > 0.0f) {
        comp.transition_progress += delta_time / comp.transition_time;
        if (comp.transition_progress >= 1.0f) {
            comp.transition_progress = 1.0f;
            comp.in_transition = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool PlayerModeSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PlayerModeState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Mode transitions
// ---------------------------------------------------------------------------

bool PlayerModeSystem::switch_mode(
        const std::string& entity_id,
        components::PlayerModeState::PlayerMode new_mode) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->in_transition) return false;
    if (new_mode == comp->current_mode) return false;

    comp->previous_mode      = comp->current_mode;
    comp->current_mode       = new_mode;
    comp->in_transition      = true;
    comp->transition_progress = 0.0f;
    comp->total_mode_switches++;
    return true;
}

bool PlayerModeSystem::complete_transition(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->in_transition) return false;
    comp->in_transition       = false;
    comp->transition_progress = 1.0f;
    return true;
}

// ---------------------------------------------------------------------------
// Binding
// ---------------------------------------------------------------------------

bool PlayerModeSystem::bind_entity(const std::string& entity_id,
                                    const std::string& target_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->bound_entity_id = target_id;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int PlayerModeSystem::get_current_mode(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->current_mode);
}

int PlayerModeSystem::get_previous_mode(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->previous_mode);
}

std::string PlayerModeSystem::get_bound_entity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->bound_entity_id;
}

bool PlayerModeSystem::is_in_transition(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->in_transition;
}

int PlayerModeSystem::get_total_switches(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_mode_switches;
}

} // namespace systems
} // namespace atlas
