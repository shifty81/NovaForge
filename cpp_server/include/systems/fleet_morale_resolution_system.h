#ifndef NOVAFORGE_SYSTEMS_FLEET_MORALE_RESOLUTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_MORALE_RESOLUTION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Resolves fleet morale crises through ideology alignment and fracture mechanics
 *
 * Manages fleet morale, crisis detection, resolution methods (Compromise,
 * AuthorityOverride, Vote, Mediation), and captain departures.
 */
class FleetMoraleResolutionSystem : public ecs::System {
public:
    explicit FleetMoraleResolutionSystem(ecs::World* world);
    ~FleetMoraleResolutionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetMoraleResolutionSystem"; }

    bool initializeFleet(const std::string& entity_id);
    bool triggerCrisis(const std::string& entity_id);
    bool resolveWithMethod(const std::string& entity_id, const std::string& method);
    bool adjustMorale(const std::string& entity_id, float amount);
    bool setIdeologyAlignment(const std::string& entity_id, float alignment);
    float getFleetMorale(const std::string& entity_id) const;
    float getIdeologyAlignment(const std::string& entity_id) const;
    bool isCrisisActive(const std::string& entity_id) const;
    int getDepartures(const std::string& entity_id) const;
    int getResolutionCount(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_MORALE_RESOLUTION_SYSTEM_H
