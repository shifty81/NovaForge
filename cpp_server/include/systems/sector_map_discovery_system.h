#ifndef NOVAFORGE_SYSTEMS_SECTOR_MAP_DISCOVERY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SECTOR_MAP_DISCOVERY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks sector/area exploration state and fog-of-war
 *
 * Manages which sectors of the star system have been discovered by the
 * player. Supports sector visibility states (hidden, partial, full),
 * discovery timestamps, and visit counts. Enables the fog-of-war map
 * overlay in the client UI.
 */
class SectorMapDiscoverySystem : public ecs::SingleComponentSystem<components::SectorMapDiscovery> {
public:
    explicit SectorMapDiscoverySystem(ecs::World* world);
    ~SectorMapDiscoverySystem() override = default;

    std::string getName() const override { return "SectorMapDiscoverySystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool discoverSector(const std::string& entity_id, const std::string& sector_id,
                        const std::string& sector_name);
    bool visitSector(const std::string& entity_id, const std::string& sector_id);
    bool setVisibility(const std::string& entity_id, const std::string& sector_id, int level);
    bool removeSector(const std::string& entity_id, const std::string& sector_id);
    int getSectorCount(const std::string& entity_id) const;
    int getVisibility(const std::string& entity_id, const std::string& sector_id) const;
    int getVisitCount(const std::string& entity_id, const std::string& sector_id) const;
    bool isSectorDiscovered(const std::string& entity_id, const std::string& sector_id) const;
    int getFullyExploredCount(const std::string& entity_id) const;
    float getExplorationPercent(const std::string& entity_id) const;
    int getTotalVisits(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SectorMapDiscovery& smd, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SECTOR_MAP_DISCOVERY_SYSTEM_H
