#ifndef NOVAFORGE_SYSTEMS_NOTIFICATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NOTIFICATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief In-game notification management system
 *
 * Sends, reads, and auto-expires player notifications.  Each notification
 * carries a type (Info, Warning, Error, Achievement, Combat, Trade), a
 * lifetime (default 60 s), and a read flag.  The list is capped at
 * max_notifications (default 100); oldest entries are purged when the cap
 * is reached.  Notifications whose age exceeds their lifetime are removed
 * on each tick.  Lifetime counters total_notifications_sent and
 * total_expired are maintained independently.
 */
class NotificationSystem
    : public ecs::SingleComponentSystem<components::NotificationState> {
public:
    explicit NotificationSystem(ecs::World* world);
    ~NotificationSystem() override = default;

    std::string getName() const override { return "NotificationSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Notification management ---
    bool sendNotification(const std::string& entity_id,
                          const std::string& notif_id,
                          const std::string& message,
                          components::NotificationState::NotifType type,
                          float lifetime);
    bool markRead(const std::string& entity_id,
                  const std::string& notif_id);
    bool markAllRead(const std::string& entity_id);
    bool removeNotification(const std::string& entity_id,
                            const std::string& notif_id);
    bool clearAll(const std::string& entity_id);

    // --- Configuration ---
    bool setMaxNotifications(const std::string& entity_id, int max_notifs);

    // --- Queries ---
    int  getNotificationCount(const std::string& entity_id) const;
    int  getUnreadCount(const std::string& entity_id) const;
    bool hasNotification(const std::string& entity_id,
                         const std::string& notif_id) const;
    bool isRead(const std::string& entity_id,
                const std::string& notif_id) const;
    int  getTotalNotificationsSent(const std::string& entity_id) const;
    int  getTotalExpired(const std::string& entity_id) const;
    int  getCountByType(const std::string& entity_id,
                        components::NotificationState::NotifType type) const;
    std::string getNotificationMessage(const std::string& entity_id,
                                       const std::string& notif_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::NotificationState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NOTIFICATION_SYSTEM_H
