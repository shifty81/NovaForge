#ifndef NOVAFORGE_SYSTEMS_FORMATION_FLIGHT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FORMATION_FLIGHT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Fleet formation slot assignment and cohesion tracking
 *
 * Each fleet member holds a formation slot with a relative position
 * offset from the leader.  The system measures how well each member
 * maintains their assigned position and applies cohesion bonuses
 * when the formation holds tight.  Broken formations lose bonuses.
 */
class FormationFlightSystem : public ecs::SingleComponentSystem<components::FormationFlight> {
public:
    explicit FormationFlightSystem(ecs::World* world);
    ~FormationFlightSystem() override = default;

    std::string getName() const override { return "FormationFlightSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& fleet_id,
                    const std::string& leader_id, int slot_index);
    bool setFormationType(const std::string& entity_id, const std::string& formation);
    bool setSlotOffset(const std::string& entity_id, double x, double y, double z);
    bool updateActualPosition(const std::string& entity_id, double x, double y, double z);
    bool setMaxDrift(const std::string& entity_id, float max_drift);
    bool reformFormation(const std::string& entity_id);

    std::string getFormationType(const std::string& entity_id) const;
    std::string getSlotStatus(const std::string& entity_id) const;
    float getCohesion(const std::string& entity_id) const;
    float getCohesionBonus(const std::string& entity_id) const;
    int getFormationBreaks(const std::string& entity_id) const;
    int getFormationReforms(const std::string& entity_id) const;
    double getDriftDistance(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FormationFlight& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FORMATION_FLIGHT_SYSTEM_H
