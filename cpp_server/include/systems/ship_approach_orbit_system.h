#ifndef NOVAFORGE_SYSTEMS_SHIP_APPROACH_ORBIT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_APPROACH_ORBIT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship approach / orbit / keep-at-range navigation commands
 *
 * Provides high-level navigation primitives used by player and NPC ships.
 * A ship can be commanded to approach a target (close distance to zero),
 * orbit a target at a specified radius, or keep at a fixed range.  Each
 * tick the system adjusts the ship's velocity toward the desired geometry.
 * Commands are overridden by new commands — only one active at a time.
 */
class ShipApproachOrbitSystem : public ecs::SingleComponentSystem<components::ApproachOrbitState> {
public:
    explicit ShipApproachOrbitSystem(ecs::World* world);
    ~ShipApproachOrbitSystem() override = default;

    std::string getName() const override { return "ShipApproachOrbitSystem"; }

public:
    bool initialize(const std::string& entity_id, float max_speed);
    bool commandApproach(const std::string& entity_id, const std::string& target_id,
                         float target_distance);
    bool commandOrbit(const std::string& entity_id, const std::string& target_id,
                      float orbit_radius);
    bool commandKeepAtRange(const std::string& entity_id, const std::string& target_id,
                            float desired_range);
    bool stopCommand(const std::string& entity_id);
    bool setMaxSpeed(const std::string& entity_id, float max_speed);

    std::string getCommandType(const std::string& entity_id) const;
    std::string getTargetId(const std::string& entity_id) const;
    float getCurrentDistance(const std::string& entity_id) const;
    float getDesiredDistance(const std::string& entity_id) const;
    float getCurrentSpeed(const std::string& entity_id) const;
    float getOrbitAngle(const std::string& entity_id) const;
    bool isCommandActive(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ApproachOrbitState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_APPROACH_ORBIT_SYSTEM_H
