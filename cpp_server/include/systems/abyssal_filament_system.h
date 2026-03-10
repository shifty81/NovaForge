#ifndef NOVAFORGE_SYSTEMS_ABYSSAL_FILAMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ABYSSAL_FILAMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Abyssal Deadspace filament entry and pocket progression system
 *
 * Activating a filament teleports the pilot into a time-limited
 * Abyssal Deadspace run consisting of up to three sequential pockets.
 * Each pocket has its own countdown timer; letting it expire marks
 * the pocket as failed.  Completing all pockets ends the run
 * successfully.
 */
class AbyssalFilamentSystem
    : public ecs::SingleComponentSystem<components::AbyssalFilamentState> {
public:
    explicit AbyssalFilamentSystem(ecs::World* world);
    ~AbyssalFilamentSystem() override = default;

    std::string getName() const override { return "AbyssalFilamentSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& pilot_id = "");
    bool activateFilament(const std::string& entity_id,
                          components::AbyssalFilamentState::FilamentType type,
                          components::AbyssalFilamentState::Tier tier);
    bool completePocket(const std::string& entity_id);
    bool cancelRun(const std::string& entity_id);

    int  getCurrentPocket(const std::string& entity_id) const;
    int  getPocketsCompleted(const std::string& entity_id) const;
    int  getPocketsFailed(const std::string& entity_id) const;
    int  getFilamentsConsumed(const std::string& entity_id) const;
    bool isActive(const std::string& entity_id) const;
    bool isRunComplete(const std::string& entity_id) const;
    float getPocketTimeRemaining(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AbyssalFilamentState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ABYSSAL_FILAMENT_SYSTEM_H
