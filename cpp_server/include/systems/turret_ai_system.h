#ifndef NOVAFORGE_SYSTEMS_TURRET_AI_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TURRET_AI_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Automated turret targeting and firing within arc constraints
 *
 * Manages turret AI for ships and stations:
 * - Each turret has a firing arc and tracking speed
 * - Turrets auto-acquire targets within their arc
 * - Priority: locked target > closest hostile > highest threat
 * - Turrets respect cooldowns and capacitor costs
 * - Tracking speed limits engagement of fast targets
 */
class TurretAISystem : public ecs::SingleComponentSystem<components::TurretAIState> {
public:
    explicit TurretAISystem(ecs::World* world);
    ~TurretAISystem() override = default;

    std::string getName() const override { return "TurretAISystem"; }

    // --- API ---

    /** Check if a bearing (degrees) is within a turret's firing arc */
    static bool isWithinArc(float bearing_deg, float turret_direction_deg,
                            float arc_degrees);

    /** Compute tracking penalty (0.0 = miss, 1.0 = full hit) based on angular velocity */
    static float computeTrackingPenalty(float angular_velocity,
                                        float tracking_speed);

    /** Get number of turrets currently engaging targets */
    int getEngagedTurretCount(const std::string& entity_id) const;

    /** Get total DPS from all active turrets on an entity */
    float getActiveDPS(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TurretAIState& turret, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TURRET_AI_SYSTEM_H
