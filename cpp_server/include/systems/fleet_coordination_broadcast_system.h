#ifndef NOVAFORGE_SYSTEMS_FLEET_COORDINATION_BROADCAST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_COORDINATION_BROADCAST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Broadcasts fleet-wide coordination signals (rally, retreat, regroup)
 *
 * Manages fleet coordination state by issuing and tracking timed signals
 * that synchronize fleet member behavior during combat, mining ops, or
 * travel. Signals decay over time and have a limited broadcast range.
 * Integrates with fleet morale and formation systems.
 */
class FleetCoordinationBroadcastSystem : public ecs::SingleComponentSystem<components::FleetCoordinationState> {
public:
    explicit FleetCoordinationBroadcastSystem(ecs::World* world);
    ~FleetCoordinationBroadcastSystem() override = default;

    std::string getName() const override { return "FleetCoordinationBroadcastSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool broadcastSignal(const std::string& entity_id, const std::string& signal_type, const std::string& issuer_id);
    bool acknowledgeSignal(const std::string& entity_id, const std::string& signal_type);
    bool clearSignals(const std::string& entity_id);
    bool setBroadcastRange(const std::string& entity_id, float range);
    bool setSignalStrength(const std::string& entity_id, float strength);
    float getBroadcastRange(const std::string& entity_id) const;
    float getSignalStrength(const std::string& entity_id) const;
    int getActiveSignalCount(const std::string& entity_id) const;
    int getTotalBroadcasts(const std::string& entity_id) const;
    int getTotalAcknowledged(const std::string& entity_id) const;
    bool hasActiveSignal(const std::string& entity_id, const std::string& signal_type) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetCoordinationState& fcs, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_COORDINATION_BROADCAST_SYSTEM_H
