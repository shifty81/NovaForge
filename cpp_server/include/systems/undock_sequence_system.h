#ifndef NOVAFORGE_SYSTEMS_UNDOCK_SEQUENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_UNDOCK_SEQUENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages the multi-phase undock animation sequence from stations
 *
 * Tracks phase progression from Docked through HangarExit, TunnelTraversal,
 * ExitAnimation to Ejected/Complete. Provides post-undock invulnerability window.
 */
class UndockSequenceSystem : public ecs::SingleComponentSystem<components::UndockSequence> {
public:
    explicit UndockSequenceSystem(ecs::World* world);
    ~UndockSequenceSystem() override = default;

    std::string getName() const override { return "UndockSequenceSystem"; }

    bool requestUndock(const std::string& entity_id, const std::string& station_id);
    bool cancelUndock(const std::string& entity_id);
    int getPhase(const std::string& entity_id) const;
    float getProgress(const std::string& entity_id) const;
    bool isUndocking(const std::string& entity_id) const;
    bool isInvulnerable(const std::string& entity_id) const;
    float getTotalUndockTime(const std::string& entity_id) const;
    int getUndockCount(const std::string& entity_id) const;
    bool setExitPosition(const std::string& entity_id, float x, float y, float z);
    bool setPhaseSpeed(const std::string& entity_id, float speed);

protected:
    void updateComponent(ecs::Entity& entity, components::UndockSequence& seq, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_UNDOCK_SEQUENCE_SYSTEM_H
