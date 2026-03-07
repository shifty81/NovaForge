#ifndef NOVAFORGE_SYSTEMS_SAFE_ZONE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SAFE_ZONE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Station safe-zone enforcement system
 *
 * Manages protection perimeters around stations and structures.
 * Tracks entity entry / exit, blocks weapon activation inside
 * the zone, and applies a tether speed bonus to sheltered ships.
 */
class SafeZoneSystem : public ecs::SingleComponentSystem<components::SafeZone> {
public:
    explicit SafeZoneSystem(ecs::World* world);
    ~SafeZoneSystem() override = default;

    std::string getName() const override { return "SafeZoneSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& zone_id,
                    const std::string& station_id);
    bool setRadius(const std::string& entity_id, float radius);
    bool enableZone(const std::string& entity_id);
    bool disableZone(const std::string& entity_id);
    bool reinforceZone(const std::string& entity_id);
    bool entityEnter(const std::string& entity_id);
    bool entityExit(const std::string& entity_id);
    bool blockWeapon(const std::string& entity_id);
    std::string getZoneState(const std::string& entity_id) const;
    float getRadius(const std::string& entity_id) const;
    int getEntitiesInside(const std::string& entity_id) const;
    bool areWeaponsDisabled(const std::string& entity_id) const;
    float getTetherBonus(const std::string& entity_id) const;
    int getTotalEntries(const std::string& entity_id) const;
    int getTotalExits(const std::string& entity_id) const;
    int getTotalWeaponsBlocked(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SafeZone& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SAFE_ZONE_SYSTEM_H
