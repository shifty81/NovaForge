#ifndef NOVAFORGE_SYSTEMS_BEACON_NAVIGATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BEACON_NAVIGATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Navigation beacon management — deploy, scan, and warp-to beacons
 *
 * Players and NPCs deploy beacons in star systems as waypoints,
 * fleet warp targets, or survey markers.  Beacons degrade over time
 * and go offline when their signal strength drops to zero.  Other
 * ships can scan for nearby beacons and warp to them.
 */
class BeaconNavigationSystem : public ecs::SingleComponentSystem<components::NavigationBeacon> {
public:
    explicit BeaconNavigationSystem(ecs::World* world);
    ~BeaconNavigationSystem() override = default;

    std::string getName() const override { return "BeaconNavigationSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& beacon_id,
                    const std::string& owner_id, const std::string& system_id);
    bool setPosition(const std::string& entity_id, double x, double y, double z);
    bool setLabel(const std::string& entity_id, const std::string& label);
    bool setType(const std::string& entity_id, const std::string& type);
    bool setPublic(const std::string& entity_id, bool is_public);
    bool recordWarpTo(const std::string& entity_id);
    bool recordScan(const std::string& entity_id);
    bool repair(const std::string& entity_id);

    std::string getState(const std::string& entity_id) const;
    std::string getLabel(const std::string& entity_id) const;
    std::string getType(const std::string& entity_id) const;
    float getSignalStrength(const std::string& entity_id) const;
    float getScanRange(const std::string& entity_id) const;
    bool isPublic(const std::string& entity_id) const;
    int getTotalWarpsTo(const std::string& entity_id) const;
    int getTotalScans(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NavigationBeacon& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BEACON_NAVIGATION_SYSTEM_H
