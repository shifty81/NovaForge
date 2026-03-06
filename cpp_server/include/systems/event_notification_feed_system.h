#ifndef NOVAFORGE_SYSTEMS_EVENT_NOTIFICATION_FEED_SYSTEM_H
#define NOVAFORGE_SYSTEMS_EVENT_NOTIFICATION_FEED_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Centralized event notification queue for player-facing gameplay feedback
 *
 * Aggregates gameplay events (combat hits, docking confirmations, ore mined,
 * market trades, warp completed, etc.) into a prioritized notification feed.
 * Handles notification lifetime, priority-based overflow eviction, and
 * read/unread state for HUD rendering.
 */
class EventNotificationFeedSystem : public ecs::SingleComponentSystem<components::EventNotificationFeed> {
public:
    explicit EventNotificationFeedSystem(ecs::World* world);
    ~EventNotificationFeedSystem() override = default;

    std::string getName() const override { return "EventNotificationFeedSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Push notifications
    bool pushNotification(const std::string& entity_id, const std::string& event_id,
                          const std::string& category, const std::string& message,
                          int priority, float lifetime);
    int getNotificationCount(const std::string& entity_id) const;
    bool hasNotification(const std::string& entity_id, const std::string& event_id) const;

    // Read / dismiss
    bool markRead(const std::string& entity_id, const std::string& event_id);
    bool dismissNotification(const std::string& entity_id, const std::string& event_id);
    int getUnreadCount(const std::string& entity_id) const;

    // Category queries
    int getCountByCategory(const std::string& entity_id, const std::string& category) const;
    std::string getLatestMessage(const std::string& entity_id) const;
    std::string getLatestMessageInCategory(const std::string& entity_id, const std::string& category) const;

    // Bulk operations
    bool clearAll(const std::string& entity_id);
    bool clearCategory(const std::string& entity_id, const std::string& category);

    // Summary
    int getTotalPushed(const std::string& entity_id) const;
    int getTotalExpired(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::EventNotificationFeed& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_EVENT_NOTIFICATION_FEED_SYSTEM_H
