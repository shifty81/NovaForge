#ifndef NOVAFORGE_SYSTEMS_ALERT_STACK_HUD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ALERT_STACK_HUD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Warning / notification alert stack for the Ship HUD
 *
 * Manages a priority-ordered stack of timed alerts (shield low,
 * cargo full, etc.).  Non-persistent alerts auto-expire; persistent
 * alerts remain until explicitly dismissed.
 */
class AlertStackHudSystem : public ecs::SingleComponentSystem<components::AlertStackHud> {
public:
    explicit AlertStackHudSystem(ecs::World* world);
    ~AlertStackHudSystem() override = default;

    std::string getName() const override { return "AlertStackHudSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id);
    int  pushAlert(const std::string& entity_id, int level,
                   const std::string& message, float lifetime = 5.0f,
                   bool persistent = false);
    bool dismissAlert(const std::string& entity_id, int alert_id);
    int  getAlertCount(const std::string& entity_id) const;
    int  getTotalShown(const std::string& entity_id) const;
    int  getTotalExpired(const std::string& entity_id) const;
    int  getTotalDismissed(const std::string& entity_id) const;
    bool clearAll(const std::string& entity_id);
    bool setVisible(const std::string& entity_id, bool vis);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AlertStackHud& stack,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ALERT_STACK_HUD_SYSTEM_H
