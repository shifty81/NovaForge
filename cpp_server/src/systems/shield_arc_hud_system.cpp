#include "systems/shield_arc_hud_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ui_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ShieldArcHudSystem::ShieldArcHudSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShieldArcHudSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ShieldArcHud& arc, float delta_time) {
    if (!arc.active) return;

    // Update critical flags
    arc.shield_critical = arc.shield_percent < arc.critical_threshold;
    arc.armor_critical  = arc.armor_percent  < arc.critical_threshold;
    arc.hull_critical   = arc.hull_percent   < arc.critical_threshold;

    // Flash timer for critical layers
    if (arc.shield_critical || arc.armor_critical || arc.hull_critical) {
        arc.flash_timer += delta_time;
        if (arc.flash_timer >= arc.flash_interval) {
            arc.flash_timer -= arc.flash_interval;
        }
    } else {
        arc.flash_timer = 0.0f;
    }
}

bool ShieldArcHudSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShieldArcHud>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ShieldArcHudSystem::setShieldPercent(const std::string& entity_id, float percent) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->shield_percent = (std::max)(0.0f, (std::min)(100.0f, percent));
    return true;
}

bool ShieldArcHudSystem::setArmorPercent(const std::string& entity_id, float percent) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->armor_percent = (std::max)(0.0f, (std::min)(100.0f, percent));
    return true;
}

bool ShieldArcHudSystem::setHullPercent(const std::string& entity_id, float percent) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->hull_percent = (std::max)(0.0f, (std::min)(100.0f, percent));
    return true;
}

float ShieldArcHudSystem::getShieldPercent(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->shield_percent : 0.0f;
}

float ShieldArcHudSystem::getArmorPercent(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->armor_percent : 0.0f;
}

float ShieldArcHudSystem::getHullPercent(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->hull_percent : 0.0f;
}

bool ShieldArcHudSystem::isShieldCritical(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->shield_critical : false;
}

bool ShieldArcHudSystem::isArmorCritical(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->armor_critical : false;
}

bool ShieldArcHudSystem::isHullCritical(const std::string& entity_id) const {
    auto* arc = getComponentFor(entity_id);
    return arc ? arc->hull_critical : false;
}

bool ShieldArcHudSystem::setVisible(const std::string& entity_id, bool vis) {
    auto* arc = getComponentFor(entity_id);
    if (!arc) return false;
    arc->visible = vis;
    return true;
}

} // namespace systems
} // namespace atlas
