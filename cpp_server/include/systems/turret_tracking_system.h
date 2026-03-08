#ifndef NOVAFORGE_SYSTEMS_TURRET_TRACKING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TURRET_TRACKING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages turret rotation, target tracking, and hit probability
 *
 * Turrets rotate toward targets at a fixed tracking speed. Hit probability
 * is computed from the ratio of tracking speed to target angular velocity,
 * with range falloff beyond optimal + falloff. Integrates with weapon and
 * damage systems.
 */
class TurretTrackingSystem : public ecs::SingleComponentSystem<components::TurretTrackingState> {
public:
    explicit TurretTrackingSystem(ecs::World* world);
    ~TurretTrackingSystem() override = default;

    std::string getName() const override { return "TurretTrackingSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& turret_id);
    bool lockTarget(const std::string& entity_id, const std::string& target_id);
    bool unlockTarget(const std::string& entity_id);
    bool setTrackingSpeed(const std::string& entity_id, float speed);
    bool setOptimalRange(const std::string& entity_id, float range);
    bool setFalloffRange(const std::string& entity_id, float range);
    bool setTargetAngularVelocity(const std::string& entity_id, float angular_vel);
    bool fireShot(const std::string& entity_id);
    float getAccuracy(const std::string& entity_id) const;
    float getDamageMultiplier(const std::string& entity_id) const;
    float getHitRate(const std::string& entity_id) const;
    bool isLocked(const std::string& entity_id) const;
    int getTotalShots(const std::string& entity_id) const;
    int getTotalHits(const std::string& entity_id) const;
    std::string getTargetId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TurretTrackingState& state, float delta_time) override;

private:
    float computeAccuracy(const components::TurretTrackingState& state) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TURRET_TRACKING_SYSTEM_H
