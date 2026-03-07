#ifndef NOVAFORGE_SYSTEMS_DOCKING_REQUEST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DOCKING_REQUEST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages the docking workflow: Idle → Approach → Requested → Granted → Docked
 *
 * Ships within docking range may request permission.  Once granted the
 * tether progress fills each tick.  At 100% the ship is Docked.
 */
class DockingRequestSystem : public ecs::SingleComponentSystem<components::DockingRequest> {
public:
    explicit DockingRequestSystem(ecs::World* world);
    ~DockingRequestSystem() override = default;

    std::string getName() const override { return "DockingRequestSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& station_id);
    bool beginApproach(const std::string& entity_id, float distance);
    bool requestDocking(const std::string& entity_id);
    bool grantDocking(const std::string& entity_id);
    bool denyDocking(const std::string& entity_id);
    bool undock(const std::string& entity_id);

    std::string getPhase(const std::string& entity_id) const;
    float getTetherProgress(const std::string& entity_id) const;
    int getTotalDockings(const std::string& entity_id) const;
    int getDeniedCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::DockingRequest& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DOCKING_REQUEST_SYSTEM_H
