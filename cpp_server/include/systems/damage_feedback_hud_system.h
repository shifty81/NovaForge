#ifndef NOVAFORGE_SYSTEMS_DAMAGE_FEEDBACK_HUD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DAMAGE_FEEDBACK_HUD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Damage feedback overlay for the Ship HUD
 *
 * Triggers and decays screen-space damage effects:
 *   - Shield ripple  (blue flash, fast decay)
 *   - Armor flash    (yellow/orange, medium decay)
 *   - Hull shake     (red + screen-shake, slow decay)
 * Each layer decays independently.  Screen shake decays at its own rate.
 */
class DamageFeedbackHudSystem : public ecs::SingleComponentSystem<components::DamageFeedbackHud> {
public:
    explicit DamageFeedbackHudSystem(ecs::World* world);
    ~DamageFeedbackHudSystem() override = default;

    std::string getName() const override { return "DamageFeedbackHudSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    bool triggerShieldHit(const std::string& entity_id, float intensity);
    bool triggerArmorHit(const std::string& entity_id, float intensity);
    bool triggerHullHit(const std::string& entity_id, float intensity);
    float getShieldIntensity(const std::string& entity_id) const;
    float getArmorIntensity(const std::string& entity_id) const;
    float getHullIntensity(const std::string& entity_id) const;
    float getScreenShake(const std::string& entity_id) const;
    int   getTotalShieldHits(const std::string& entity_id) const;
    int   getTotalArmorHits(const std::string& entity_id) const;
    int   getTotalHullHits(const std::string& entity_id) const;
    bool  setVisible(const std::string& entity_id, bool vis);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DamageFeedbackHud& fb,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DAMAGE_FEEDBACK_HUD_SYSTEM_H
