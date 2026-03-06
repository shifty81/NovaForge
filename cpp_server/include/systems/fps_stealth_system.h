#ifndef NOVAFORGE_SYSTEMS_FPS_STEALTH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_STEALTH_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Detection, vision, and stealth mechanics for FPS gameplay
 *
 * Manages visibility, noise level, detection meter, and alert state transitions.
 * Supports crouch/sprint modifiers and shadow zones.
 */
class FPSStealthSystem : public ecs::SingleComponentSystem<components::FPSStealth> {
public:
    explicit FPSStealthSystem(ecs::World* world);
    ~FPSStealthSystem() override = default;

    std::string getName() const override { return "FPSStealthSystem"; }

    int getDetectionState(const std::string& entity_id) const;
    float getDetectionMeter(const std::string& entity_id) const;
    bool addDetection(const std::string& entity_id, float amount);
    bool setCrouching(const std::string& entity_id, bool crouching);
    bool setSprinting(const std::string& entity_id, bool sprinting);
    bool setInShadow(const std::string& entity_id, bool in_shadow);
    bool setLightLevel(const std::string& entity_id, float level);
    float getVisibility(const std::string& entity_id) const;
    float getNoiseLevel(const std::string& entity_id) const;
    int getTimesDetected(const std::string& entity_id) const;
    int getTimesEscaped(const std::string& entity_id) const;
    float getTimeHidden(const std::string& entity_id) const;
    bool resetDetection(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::FPSStealth& stealth, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_STEALTH_SYSTEM_H
