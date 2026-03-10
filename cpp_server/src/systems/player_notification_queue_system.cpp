#include "systems/player_notification_queue_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlayerNotificationQueueSystem::PlayerNotificationQueueSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void PlayerNotificationQueueSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::PlayerNotificationQueue& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Age-out expired notifications
    auto it = comp.notifications.begin();
    while (it != comp.notifications.end()) {
        float age = comp.elapsed - it->timestamp;
        if (age >= it->lifetime) {
            ++comp.total_expired;
            it = comp.notifications.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool PlayerNotificationQueueSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    entity->addComponent(
        std::make_unique<components::PlayerNotificationQueue>());
    return true;
}

// ---------------------------------------------------------------------------
// Queue management
// ---------------------------------------------------------------------------

bool PlayerNotificationQueueSystem::push(
        const std::string& entity_id,
        const std::string& notif_id,
        components::PlayerNotificationQueue::NotificationType type,
        const std::string& message,
        float lifetime) {
    if (notif_id.empty() || message.empty() || lifetime <= 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Evict oldest if at capacity
    if (static_cast<int>(comp->notifications.size()) >= comp->max_notifications) {
        comp->notifications.erase(comp->notifications.begin());
    }

    components::PlayerNotificationQueue::Notification n;
    n.id        = notif_id;
    n.type      = type;
    n.message   = message;
    n.timestamp = comp->elapsed;
    n.lifetime  = lifetime;
    n.read      = false;

    comp->notifications.push_back(n);
    ++comp->total_pushed;
    return true;
}

bool PlayerNotificationQueueSystem::markRead(const std::string& entity_id,
                                              const std::string& notif_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& n : comp->notifications) {
        if (n.id == notif_id) { n.read = true; return true; }
    }
    return false;
}

bool PlayerNotificationQueueSystem::markAllRead(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& n : comp->notifications) { n.read = true; }
    return true;
}

bool PlayerNotificationQueueSystem::clearNotifications(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->notifications.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int PlayerNotificationQueueSystem::getTotalCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->notifications.size()) : 0;
}

int PlayerNotificationQueueSystem::getUnreadCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& n : comp->notifications) {
        if (!n.read) ++count;
    }
    return count;
}

int PlayerNotificationQueueSystem::getTotalPushed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_pushed : 0;
}

int PlayerNotificationQueueSystem::getTotalExpired(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_expired : 0;
}

components::PlayerNotificationQueue::Notification
PlayerNotificationQueueSystem::getMostRecentUnread(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return {};
    const components::PlayerNotificationQueue::Notification* best = nullptr;
    for (const auto& n : comp->notifications) {
        if (!n.read) {
            if (!best || n.timestamp > best->timestamp) {
                best = &n;
            }
        }
    }
    return best ? *best : components::PlayerNotificationQueue::Notification{};
}

int PlayerNotificationQueueSystem::getCountByType(
        const std::string& entity_id,
        components::PlayerNotificationQueue::NotificationType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& n : comp->notifications) {
        if (n.type == type) ++count;
    }
    return count;
}

} // namespace systems
} // namespace atlas
