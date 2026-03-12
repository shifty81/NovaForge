#ifndef NOVAFORGE_SYSTEMS_FLEET_BROADCAST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_BROADCAST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet broadcast message management system
 *
 * Tracks broadcast messages used for fleet coordination.  Each broadcast
 * has a type (Target, AlignTo, WarpTo, NeedShieldReps, NeedArmorReps,
 * NeedCapacitor, EnemySpotted, HoldPosition), a sender, a target label,
 * and a TTL countdown.  Broadcasts expire when TTL reaches 0 and are
 * removed automatically.  The active list is capped at max_broadcasts
 * (default 20).
 */
class FleetBroadcastSystem
    : public ecs::SingleComponentSystem<components::FleetBroadcastState> {
public:
    explicit FleetBroadcastSystem(ecs::World* world);
    ~FleetBroadcastSystem() override = default;

    std::string getName() const override { return "FleetBroadcastSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    const std::string& fleet_id = "");

    // --- Broadcast management ---
    bool sendBroadcast(const std::string& entity_id,
                       const std::string& broadcast_id,
                       components::FleetBroadcastState::BroadcastType type,
                       const std::string& sender_id,
                       const std::string& target_label,
                       float ttl = 30.0f);
    bool dismissBroadcast(const std::string& entity_id,
                          const std::string& broadcast_id);
    bool clearBroadcasts(const std::string& entity_id);

    // --- Configuration ---
    bool setFleetId(const std::string& entity_id,
                    const std::string& fleet_id);

    // --- Queries ---
    int         getActiveBroadcastCount(const std::string& entity_id) const;
    int         getTotalSent(const std::string& entity_id) const;
    int         getTotalExpired(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;
    bool        hasBroadcast(const std::string& entity_id,
                             const std::string& broadcast_id) const;
    int         getCountByType(
                    const std::string& entity_id,
                    components::FleetBroadcastState::BroadcastType type) const;
    float       getTtl(const std::string& entity_id,
                       const std::string& broadcast_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetBroadcastState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_BROADCAST_SYSTEM_H
