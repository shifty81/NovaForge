#ifndef NOVAFORGE_SYSTEMS_PLAYER_NOTIFICATION_QUEUE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_NOTIFICATION_QUEUE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Generic player-facing notification queue
 *
 * Provides a typed, ordered queue of notifications the player should be
 * shown: mission updates, trade confirmations, kill mail arrivals, system
 * alerts, and general game events.  Each notification has a configurable
 * lifetime; expired entries are pruned on every tick.  When the queue is
 * full the oldest entry is evicted to make room for the new one.
 *
 * Notifications are marked read individually or all-at-once.
 * getUnreadCount() returns the number of unread entries.
 * getMostRecentUnread() returns the newest unread notification by
 * timestamp, or an empty default if all are read.
 */
class PlayerNotificationQueueSystem
    : public ecs::SingleComponentSystem<components::PlayerNotificationQueue> {
public:
    explicit PlayerNotificationQueueSystem(ecs::World* world);
    ~PlayerNotificationQueueSystem() override = default;

    std::string getName() const override {
        return "PlayerNotificationQueueSystem";
    }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Queue management ---
    bool push(const std::string& entity_id,
              const std::string& notif_id,
              components::PlayerNotificationQueue::NotificationType type,
              const std::string& message,
              float lifetime = 60.0f);

    bool markRead(const std::string& entity_id,
                  const std::string& notif_id);
    bool markAllRead(const std::string& entity_id);
    bool clearNotifications(const std::string& entity_id);

    // --- Queries ---
    int  getTotalCount(const std::string& entity_id) const;
    int  getUnreadCount(const std::string& entity_id) const;
    int  getTotalPushed(const std::string& entity_id) const;
    int  getTotalExpired(const std::string& entity_id) const;
    components::PlayerNotificationQueue::Notification
         getMostRecentUnread(const std::string& entity_id) const;
    int  getCountByType(
             const std::string& entity_id,
             components::PlayerNotificationQueue::NotificationType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PlayerNotificationQueue& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_NOTIFICATION_QUEUE_SYSTEM_H
