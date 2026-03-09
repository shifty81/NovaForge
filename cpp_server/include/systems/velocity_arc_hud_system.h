#ifndef NOVAFORGE_SYSTEMS_VELOCITY_ARC_HUD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VELOCITY_ARC_HUD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Velocity arc indicator for the Ship HUD Control Ring
 *
 * Displays current speed as a sweeping arc with colour-coded states:
 *   Idle (< 1 % max), Normal, Approaching (> 80 %), AtMax (100 %).
 * Also tracks afterburner state and warp charge-up progress.
 */
class VelocityArcHudSystem : public ecs::SingleComponentSystem<components::VelocityArcHud> {
public:
    explicit VelocityArcHudSystem(ecs::World* world);
    ~VelocityArcHudSystem() override = default;

    std::string getName() const override { return "VelocityArcHudSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    bool setSpeed(const std::string& entity_id, float current, float max_speed);
    bool setAfterburner(const std::string& entity_id, bool on);
    bool setWarpPrepProgress(const std::string& entity_id, float progress);
    float getSpeedPercent(const std::string& entity_id) const;
    int   getSpeedState(const std::string& entity_id) const;
    bool  isAfterburnerActive(const std::string& entity_id) const;
    float getWarpPrepProgress(const std::string& entity_id) const;
    bool  setVisible(const std::string& entity_id, bool vis);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::VelocityArcHud& arc,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VELOCITY_ARC_HUD_SYSTEM_H
