#ifndef NOVAFORGE_SYSTEMS_SHIELD_ARC_HUD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIELD_ARC_HUD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Renders circular shield/armor/hull arcs for the Ship HUD Control Ring
 *
 * Three concentric arcs (shield outermost, hull innermost) deplete
 * clockwise from 12 o'clock.  When any layer drops below 25 % the
 * corresponding arc enters a critical flash state.
 */
class ShieldArcHudSystem : public ecs::SingleComponentSystem<components::ShieldArcHud> {
public:
    explicit ShieldArcHudSystem(ecs::World* world);
    ~ShieldArcHudSystem() override = default;

    std::string getName() const override { return "ShieldArcHudSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    bool setShieldPercent(const std::string& entity_id, float percent);
    bool setArmorPercent(const std::string& entity_id, float percent);
    bool setHullPercent(const std::string& entity_id, float percent);
    float getShieldPercent(const std::string& entity_id) const;
    float getArmorPercent(const std::string& entity_id) const;
    float getHullPercent(const std::string& entity_id) const;
    bool isShieldCritical(const std::string& entity_id) const;
    bool isArmorCritical(const std::string& entity_id) const;
    bool isHullCritical(const std::string& entity_id) const;
    bool setVisible(const std::string& entity_id, bool vis);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::ShieldArcHud& arc,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIELD_ARC_HUD_SYSTEM_H
