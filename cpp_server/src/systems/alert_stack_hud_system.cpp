#include "systems/alert_stack_hud_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ui_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

AlertStackHudSystem::AlertStackHudSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AlertStackHudSystem::updateComponent(ecs::Entity& /*entity*/,
    components::AlertStackHud& stack, float delta_time) {
    if (!stack.active) return;

    // Tick lifetimes and expire non-persistent alerts
    for (auto it = stack.alerts.begin(); it != stack.alerts.end(); ) {
        if (it->dismissed) {
            ++it;
            continue;
        }
        if (!it->persistent) {
            it->lifetime -= delta_time;
            if (it->lifetime <= 0.0f) {
                stack.total_expired++;
                it = stack.alerts.erase(it);
                continue;
            }
        }
        ++it;
    }

    // Remove dismissed alerts
    stack.alerts.erase(
        std::remove_if(stack.alerts.begin(), stack.alerts.end(),
            [](const components::AlertStackHud::Alert& a) { return a.dismissed; }),
        stack.alerts.end());
}

bool AlertStackHudSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AlertStackHud>();
    entity->addComponent(std::move(comp));
    return true;
}

int AlertStackHudSystem::pushAlert(const std::string& entity_id, int level,
    const std::string& message, float lifetime, bool persistent) {
    auto* stack = getComponentFor(entity_id);
    if (!stack) return -1;
    if (static_cast<int>(stack->alerts.size()) >= stack->max_alerts) return -1;

    components::AlertStackHud::Alert alert;
    alert.id           = stack->next_alert_id++;
    alert.level        = level;
    alert.message      = message;
    alert.lifetime     = lifetime;
    alert.max_lifetime = lifetime;
    alert.persistent   = persistent;
    alert.dismissed    = false;

    stack->alerts.push_back(alert);
    stack->total_shown++;
    return alert.id;
}

bool AlertStackHudSystem::dismissAlert(const std::string& entity_id, int alert_id) {
    auto* stack = getComponentFor(entity_id);
    if (!stack) return false;

    auto* alert = stack->findAlert(alert_id);
    if (!alert || alert->dismissed) return false;
    alert->dismissed = true;
    stack->total_dismissed++;
    return true;
}

int AlertStackHudSystem::getAlertCount(const std::string& entity_id) const {
    auto* stack = getComponentFor(entity_id);
    if (!stack) return 0;
    int count = 0;
    for (const auto& a : stack->alerts) {
        if (!a.dismissed) count++;
    }
    return count;
}

int AlertStackHudSystem::getTotalShown(const std::string& entity_id) const {
    auto* stack = getComponentFor(entity_id);
    return stack ? stack->total_shown : 0;
}

int AlertStackHudSystem::getTotalExpired(const std::string& entity_id) const {
    auto* stack = getComponentFor(entity_id);
    return stack ? stack->total_expired : 0;
}

int AlertStackHudSystem::getTotalDismissed(const std::string& entity_id) const {
    auto* stack = getComponentFor(entity_id);
    return stack ? stack->total_dismissed : 0;
}

bool AlertStackHudSystem::clearAll(const std::string& entity_id) {
    auto* stack = getComponentFor(entity_id);
    if (!stack) return false;
    stack->alerts.clear();
    return true;
}

bool AlertStackHudSystem::setVisible(const std::string& entity_id, bool vis) {
    auto* stack = getComponentFor(entity_id);
    if (!stack) return false;
    stack->visible = vis;
    return true;
}

} // namespace systems
} // namespace atlas
