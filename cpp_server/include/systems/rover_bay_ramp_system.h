#ifndef NOVAFORGE_SYSTEMS_ROVER_BAY_RAMP_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ROVER_BAY_RAMP_SYSTEM_H

#include "ecs/system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Rover bay ramp system (Phase 13/14)
 *
 * Manages belly hangar with folding ramp for rover deployment including
 * atmosphere safety checks, bay pressurization, and rover tracking.
 */
class RoverBayRampSystem : public ecs::System {
public:
    explicit RoverBayRampSystem(ecs::World* world);
    ~RoverBayRampSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "RoverBayRampSystem"; }

    // Initialization
    bool initializeBay(const std::string& entity_id, int max_rovers);

    // Ramp operations
    bool openRamp(const std::string& entity_id);
    bool closeRamp(const std::string& entity_id);

    // Rover operations
    bool storeRover(const std::string& entity_id, const std::string& rover_id);
    bool deployRover(const std::string& entity_id, const std::string& rover_id);
    bool retrieveRover(const std::string& entity_id, const std::string& rover_id);

    // Environment
    bool setExternalAtmosphere(const std::string& entity_id,
                               components::RoverBayRamp::AtmosphereType atmosphere);
    bool setExternalGravity(const std::string& entity_id, float gravity);
    bool setPowerEnabled(const std::string& entity_id, bool enabled);

    // Query
    std::string getRampState(const std::string& entity_id) const;
    float getRampProgress(const std::string& entity_id) const;
    int getStoredCount(const std::string& entity_id) const;
    int getDeployedCount(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ROVER_BAY_RAMP_SYSTEM_H
