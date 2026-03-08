#include "systems/jump_fatigue_tracker_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

JumpFatigueTrackerSystem::JumpFatigueTrackerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void JumpFatigueTrackerSystem::updateComponent(ecs::Entity& /*entity*/,
    components::JumpFatigueTrackerState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.blue_timer > 0.0f) {
        comp.blue_timer -= comp.decay_rate * delta_time;
        if (comp.blue_timer < 0.0f) comp.blue_timer = 0.0f;
    }

    if (comp.orange_timer > 0.0f) {
        comp.orange_timer -= comp.decay_rate * delta_time;
        if (comp.orange_timer < 0.0f) comp.orange_timer = 0.0f;
    }

    if (comp.jump_restricted && comp.orange_timer <= 300.0f) {
        comp.jump_restricted = false;
    }
}

bool JumpFatigueTrackerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::JumpFatigueTrackerState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool JumpFatigueTrackerSystem::recordJump(const std::string& entity_id, float distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (distance <= 0.0f) return false;

    float fatigue = distance * comp->fatigue_multiplier;

    comp->blue_timer += fatigue;
    if (comp->blue_timer > comp->max_blue_timer)
        comp->blue_timer = comp->max_blue_timer;

    comp->orange_timer += fatigue;
    if (comp->orange_timer > comp->max_orange_timer)
        comp->orange_timer = comp->max_orange_timer;

    comp->total_jumps++;

    if (comp->orange_timer > 300.0f && !comp->jump_restricted) {
        comp->jump_restricted = true;
        comp->total_fatigue_penalties++;
    }

    return true;
}

bool JumpFatigueTrackerSystem::setFatigueMultiplier(const std::string& entity_id,
    float multiplier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (multiplier <= 0.0f) return false;
    comp->fatigue_multiplier = multiplier;
    return true;
}

bool JumpFatigueTrackerSystem::setDecayRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate <= 0.0f) return false;
    comp->decay_rate = rate;
    return true;
}

bool JumpFatigueTrackerSystem::resetTimers(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->blue_timer = 0.0f;
    comp->orange_timer = 0.0f;
    comp->jump_restricted = false;
    return true;
}

float JumpFatigueTrackerSystem::getBlueTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->blue_timer : 0.0f;
}

float JumpFatigueTrackerSystem::getOrangeTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->orange_timer : 0.0f;
}

bool JumpFatigueTrackerSystem::isJumpRestricted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->jump_restricted : false;
}

float JumpFatigueTrackerSystem::getFatigueMultiplier(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fatigue_multiplier : 0.0f;
}

float JumpFatigueTrackerSystem::getDecayRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->decay_rate : 0.0f;
}

int JumpFatigueTrackerSystem::getTotalJumps(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jumps : 0;
}

int JumpFatigueTrackerSystem::getTotalFatiguePenalties(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_fatigue_penalties : 0;
}

} // namespace systems
} // namespace atlas
