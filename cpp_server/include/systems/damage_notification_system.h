#ifndef NOVAFORGE_SYSTEMS_DAMAGE_NOTIFICATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DAMAGE_NOTIFICATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks and queues damage notifications for HUD display
 *
 * Records incoming/outgoing damage events with type, weapon, and crit info.
 * Entries expire after configurable lifetime. Provides DPS calculations.
 */
class DamageNotificationSystem : public ecs::SingleComponentSystem<components::DamageNotification> {
public:
    explicit DamageNotificationSystem(ecs::World* world);
    ~DamageNotificationSystem() override = default;

    std::string getName() const override { return "DamageNotificationSystem"; }

    bool recordIncoming(const std::string& entity_id, const std::string& attacker_id,
                        float amount, int damage_type, const std::string& weapon, bool is_crit);
    bool recordOutgoing(const std::string& entity_id, const std::string& target_id,
                        float amount, int damage_type, const std::string& weapon, bool is_crit);
    int getIncomingCount(const std::string& entity_id) const;
    int getOutgoingCount(const std::string& entity_id) const;
    float getTotalDamageTaken(const std::string& entity_id) const;
    float getTotalDamageDealt(const std::string& entity_id) const;
    int getCritsTaken(const std::string& entity_id) const;
    int getCritsDealt(const std::string& entity_id) const;
    float getRecentDPS(const std::string& entity_id, float window) const;
    bool clearNotifications(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::DamageNotification& notif, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DAMAGE_NOTIFICATION_SYSTEM_H
