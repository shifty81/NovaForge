#ifndef NOVAFORGE_SYSTEMS_PLANET_SCAN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLANET_SCAN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Planetary scan system for Planetary Operations
 *
 * Manages the lifecycle of planet-scan probes: launch, timed
 * progress, result confirmation, and scan reset.  A completed scan
 * populates a list of resource types and richness values that the
 * player can then use to site a planetary colony.
 */
class PlanetScanSystem
    : public ecs::SingleComponentSystem<components::PlanetScanState> {
public:
    explicit PlanetScanSystem(ecs::World* world);
    ~PlanetScanSystem() override = default;

    std::string getName() const override { return "PlanetScanSystem"; }

    // --- public API ---

    /** Attach a PlanetScanState component to an existing entity. */
    bool initialize(const std::string& entity_id,
                    const std::string& planet_id,
                    const std::string& planet_type);

    /** Launch scan probes with a given strength (0–100). */
    bool launchScan(const std::string& entity_id, float probe_strength);

    /** Cancel an in-progress scan. */
    bool cancelScan(const std::string& entity_id);

    /** Confirm (accept) a specific result after scanning. */
    bool confirmResult(const std::string& entity_id,
                       const std::string& resource_type);

    bool isScanning(const std::string& entity_id) const;
    float getScanProgress(const std::string& entity_id) const;
    int   getResultCount(const std::string& entity_id) const;
    int   getProbesLaunched(const std::string& entity_id) const;
    int   getTotalScansCompleted(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PlanetScanState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLANET_SCAN_SYSTEM_H
