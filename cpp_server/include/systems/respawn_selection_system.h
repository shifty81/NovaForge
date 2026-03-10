#ifndef NOVAFORGE_SYSTEMS_RESPAWN_SELECTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RESPAWN_SELECTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Respawn location selection system
 *
 * Presents available respawn locations to a dead player.  While the panel
 * is open the system ticks the auto_select_timer down; when it reaches
 * zero the first default location (or the first location in the list) is
 * selected automatically and the panel is closed.
 */
class RespawnSelectionSystem
    : public ecs::SingleComponentSystem<components::RespawnSelection> {
public:
    explicit RespawnSelectionSystem(ecs::World* world);
    ~RespawnSelectionSystem() override = default;

    std::string getName() const override { return "RespawnSelectionSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Panel control ---
    bool openSelection(const std::string& entity_id);
    bool confirmSelection(const std::string& entity_id);

    // --- Location management ---
    bool addLocation(const std::string& entity_id,
                     const std::string& location_id,
                     const std::string& location_name,
                     float distance_ly,
                     bool is_default);
    bool removeLocation(const std::string& entity_id,
                        const std::string& location_id);
    bool selectLocation(const std::string& entity_id,
                        const std::string& location_id);

    // --- Queries ---
    bool isOpen(const std::string& entity_id) const;
    std::string getSelectedLocation(const std::string& entity_id) const;
    int  getLocationCount(const std::string& entity_id) const;
    int  getTotalSelections(const std::string& entity_id) const;

    // --- Configuration ---
    bool setAutoSelectDuration(const std::string& entity_id, float duration);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RespawnSelection& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RESPAWN_SELECTION_SYSTEM_H
