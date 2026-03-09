#include "systems/capacitor_hud_bar_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ui_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

CapacitorHudBarSystem::CapacitorHudBarSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CapacitorHudBarSystem::updateComponent(ecs::Entity& /*entity*/,
    components::CapacitorHudBar& bar, float /*delta_time*/) {
    if (!bar.active) return;

    // Derive percent
    bar.percent = (bar.maximum > 0.0f)
        ? (bar.current / bar.maximum) * 100.0f
        : 0.0f;
    bar.percent = (std::max)(0.0f, (std::min)(100.0f, bar.percent));

    // Derive color state
    if (bar.percent > bar.green_threshold) {
        bar.color_state = 0; // Green
    } else if (bar.percent > bar.yellow_threshold) {
        bar.color_state = 1; // Yellow
    } else {
        bar.color_state = 2; // Red
    }

    // Warning
    bar.warning_active = bar.percent <= bar.yellow_threshold;
}

bool CapacitorHudBarSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CapacitorHudBar>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CapacitorHudBarSystem::setCapacitor(const std::string& entity_id,
    float current, float maximum) {
    auto* bar = getComponentFor(entity_id);
    if (!bar) return false;
    bar->maximum = (std::max)(0.0f, maximum);
    bar->current = (std::max)(0.0f, (std::min)(bar->maximum, current));
    return true;
}

bool CapacitorHudBarSystem::setDrainRate(const std::string& entity_id, float rate) {
    auto* bar = getComponentFor(entity_id);
    if (!bar) return false;
    bar->drain_rate = rate;
    return true;
}

float CapacitorHudBarSystem::getPercent(const std::string& entity_id) const {
    auto* bar = getComponentFor(entity_id);
    return bar ? bar->percent : 0.0f;
}

int CapacitorHudBarSystem::getColorState(const std::string& entity_id) const {
    auto* bar = getComponentFor(entity_id);
    return bar ? bar->color_state : 0;
}

bool CapacitorHudBarSystem::isWarningActive(const std::string& entity_id) const {
    auto* bar = getComponentFor(entity_id);
    return bar ? bar->warning_active : false;
}

float CapacitorHudBarSystem::getDrainRate(const std::string& entity_id) const {
    auto* bar = getComponentFor(entity_id);
    return bar ? bar->drain_rate : 0.0f;
}

bool CapacitorHudBarSystem::setVisible(const std::string& entity_id, bool vis) {
    auto* bar = getComponentFor(entity_id);
    if (!bar) return false;
    bar->visible = vis;
    return true;
}

} // namespace systems
} // namespace atlas
