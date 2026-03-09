#include "systems/velocity_arc_hud_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ui_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

VelocityArcHudSystem::VelocityArcHudSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void VelocityArcHudSystem::updateComponent(ecs::Entity& /*entity*/,
    components::VelocityArcHud& arc, float /*delta_time*/) {
    if (!arc.active) return;

    // Derive percent
    arc.speed_percent = (arc.max_speed > 0.0f)
        ? (arc.current_speed / arc.max_speed) * 100.0f
        : 0.0f;
    arc.speed_percent = (std::max)(0.0f, (std::min)(100.0f, arc.speed_percent));

    // Derive state
    if (arc.current_speed < arc.idle_threshold) {
        arc.speed_state = 0; // Idle
    } else if (arc.speed_percent >= 100.0f) {
        arc.speed_state = 3; // AtMax
    } else if (arc.speed_percent >= arc.approach_threshold) {
        arc.speed_state = 2; // Approaching
    } else {
        arc.speed_state = 1; // Normal
    }
}

bool VelocityArcHudSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::VelocityArcHud>();
    entity->addComponent(std::move(comp));
    return true;
}

bool VelocityArcHudSystem::setSpeed(const std::string& entity_id,
    float current, float max_speed) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->max_speed = (std::max)(0.0f, max_speed);
    arc->current_speed = (std::max)(0.0f, (std::min)(arc->max_speed, current));
    return true;
}

bool VelocityArcHudSystem::setAfterburner(const std::string& entity_id, bool on) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->afterburner_active = on;
    return true;
}

bool VelocityArcHudSystem::setWarpPrepProgress(const std::string& entity_id,
    float progress) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->warp_prep_progress = (std::max)(0.0f, (std::min)(1.0f, progress));
    return true;
}

float VelocityArcHudSystem::getSpeedPercent(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->speed_percent : 0.0f;
}

int VelocityArcHudSystem::getSpeedState(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->speed_state : 0;
}

bool VelocityArcHudSystem::isAfterburnerActive(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->afterburner_active : false;
}

float VelocityArcHudSystem::getWarpPrepProgress(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->warp_prep_progress : 0.0f;
}

bool VelocityArcHudSystem::setVisible(const std::string& entity_id, bool vis) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->visible = vis;
    return true;
}

} // namespace systems
} // namespace atlas
