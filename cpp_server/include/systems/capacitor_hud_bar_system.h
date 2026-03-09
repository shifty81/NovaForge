#ifndef NOVAFORGE_SYSTEMS_CAPACITOR_HUD_BAR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPACITOR_HUD_BAR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Vertical capacitor bar for the Ship HUD Control Ring
 *
 * Displays a vertical bar that changes colour as charge is consumed:
 * green (> 50 %), yellow (25-50 %), red (< 25 %).
 * Tracks drain rate and activates a low-cap warning.
 */
class CapacitorHudBarSystem : public ecs::SingleComponentSystem<components::CapacitorHudBar> {
public:
    explicit CapacitorHudBarSystem(ecs::World* world);
    ~CapacitorHudBarSystem() override = default;

    std::string getName() const override { return "CapacitorHudBarSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    bool setCapacitor(const std::string& entity_id, float current, float maximum);
    bool setDrainRate(const std::string& entity_id, float rate);
    float getPercent(const std::string& entity_id) const;
    int   getColorState(const std::string& entity_id) const;
    bool  isWarningActive(const std::string& entity_id) const;
    float getDrainRate(const std::string& entity_id) const;
    bool  setVisible(const std::string& entity_id, bool vis);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CapacitorHudBar& bar,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPACITOR_HUD_BAR_SYSTEM_H
