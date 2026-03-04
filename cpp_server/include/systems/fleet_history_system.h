#ifndef NOVAFORGE_SYSTEMS_FLEET_HISTORY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_HISTORY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Records fleet history events for save-file persistence
 */
class FleetHistorySystem : public ecs::SingleComponentSystem<components::FleetHistory> {
public:
    explicit FleetHistorySystem(ecs::World* world);
    ~FleetHistorySystem() override = default;

    std::string getName() const override { return "FleetHistorySystem"; }

    // --- API ---
    void recordEvent(const std::string& fleet_id, const std::string& type,
                     const std::string& desc, float timestamp,
                     const std::string& entity_id = "");
    std::vector<components::FleetHistoryEntry> getHistory(const std::string& fleet_id, int count = 20) const;
    int getEventCount(const std::string& fleet_id) const;
    std::vector<components::FleetHistoryEntry> getEventsByType(const std::string& fleet_id, const std::string& type) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetHistory& history, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_HISTORY_SYSTEM_H
