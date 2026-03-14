#include "systems/notification_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

NotificationSystem::NotificationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — expire notifications whose age exceeds lifetime
// ---------------------------------------------------------------------------

void NotificationSystem::updateComponent(ecs::Entity& /*entity*/,
                                          components::NotificationState& comp,
                                          float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Remove expired notifications
    auto it = comp.notifications.begin();
    while (it != comp.notifications.end()) {
        float age = comp.elapsed - it->timestamp;
        if (age > it->lifetime) {
            it = comp.notifications.erase(it);
            comp.total_expired++;
        } else {
            ++it;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool NotificationSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NotificationState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Notification management
// ---------------------------------------------------------------------------

bool NotificationSystem::sendNotification(
        const std::string& entity_id,
        const std::string& notif_id,
        const std::string& message,
        components::NotificationState::NotifType type,
        float lifetime) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (notif_id.empty()) return false;
    if (message.empty()) return false;
    if (lifetime <= 0.0f) return false;

    // Duplicate check
    for (const auto& n : comp->notifications) {
        if (n.notif_id == notif_id) return false;
    }

    // Purge oldest if at capacity
    if (static_cast<int>(comp->notifications.size()) >= comp->max_notifications) {
        comp->notifications.erase(comp->notifications.begin());
    }

    components::NotificationState::Notification notif;
    notif.notif_id  = notif_id;
    notif.message   = message;
    notif.type      = type;
    notif.lifetime  = lifetime;
    notif.timestamp = comp->elapsed;
    notif.read      = false;
    comp->notifications.push_back(notif);

    comp->total_notifications_sent++;
    return true;
}

bool NotificationSystem::markRead(const std::string& entity_id,
                                   const std::string& notif_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& n : comp->notifications) {
        if (n.notif_id == notif_id) {
            n.read = true;
            return true;
        }
    }
    return false;
}

bool NotificationSystem::markAllRead(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& n : comp->notifications) {
        n.read = true;
    }
    return true;
}

bool NotificationSystem::removeNotification(const std::string& entity_id,
                                             const std::string& notif_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->notifications.begin(), comp->notifications.end(),
        [&](const components::NotificationState::Notification& n) {
            return n.notif_id == notif_id;
        });
    if (it == comp->notifications.end()) return false;
    comp->notifications.erase(it);
    return true;
}

bool NotificationSystem::clearAll(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->notifications.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool NotificationSystem::setMaxNotifications(const std::string& entity_id,
                                              int max_notifs) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_notifs <= 0) return false;
    comp->max_notifications = max_notifs;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int NotificationSystem::getNotificationCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->notifications.size()) : 0;
}

int NotificationSystem::getUnreadCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& n : comp->notifications) {
        if (!n.read) count++;
    }
    return count;
}

bool NotificationSystem::hasNotification(const std::string& entity_id,
                                          const std::string& notif_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& n : comp->notifications) {
        if (n.notif_id == notif_id) return true;
    }
    return false;
}

bool NotificationSystem::isRead(const std::string& entity_id,
                                 const std::string& notif_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& n : comp->notifications) {
        if (n.notif_id == notif_id) return n.read;
    }
    return false;
}

int NotificationSystem::getTotalNotificationsSent(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_notifications_sent : 0;
}

int NotificationSystem::getTotalExpired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_expired : 0;
}

int NotificationSystem::getCountByType(
        const std::string& entity_id,
        components::NotificationState::NotifType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& n : comp->notifications) {
        if (n.type == type) count++;
    }
    return count;
}

std::string NotificationSystem::getNotificationMessage(
        const std::string& entity_id,
        const std::string& notif_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& n : comp->notifications) {
        if (n.notif_id == notif_id) return n.message;
    }
    return "";
}

} // namespace systems
} // namespace atlas
