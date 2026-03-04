#ifndef NOVAFORGE_SYSTEMS_DOCKING_RING_EXTENSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DOCKING_RING_EXTENSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Docking ring extension system (Phase 13)
 *
 * Manages visual docking ring modules for ship-to-ship docking with
 * extension state machine, alignment tracking, and pressure sealing.
 */
class DockingRingExtensionSystem : public ecs::SingleComponentSystem<components::DockingRingExtension> {
public:
    explicit DockingRingExtensionSystem(ecs::World* world);
    ~DockingRingExtensionSystem() override = default;

    std::string getName() const override { return "DockingRingExtensionSystem"; }

    // Initialization
    bool initializeRing(const std::string& entity_id, float ring_diameter);

    // Ring operations
    bool extendRing(const std::string& entity_id);
    bool retractRing(const std::string& entity_id);
    bool connectRing(const std::string& entity_id, const std::string& target_entity_id,
                     components::DockingRingExtension::ConnectionType connection_type);
    bool disconnectRing(const std::string& entity_id);
    bool sealPressure(const std::string& entity_id);
    bool unsealPressure(const std::string& entity_id);

    // Configuration
    bool setAlignment(const std::string& entity_id, float angle);
    bool setPowerEnabled(const std::string& entity_id, bool enabled);
    bool repairRing(const std::string& entity_id, float amount);

    // Query
    std::string getState(const std::string& entity_id) const;
    float getProgress(const std::string& entity_id) const;
    bool isConnected(const std::string& entity_id) const;
    float getIntegrity(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::DockingRingExtension& ring, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DOCKING_RING_EXTENSION_SYSTEM_H
