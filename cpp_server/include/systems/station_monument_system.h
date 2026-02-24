#ifndef NOVAFORGE_SYSTEMS_STATION_MONUMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STATION_MONUMENT_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

// Minimum legend score required to earn a monument
static constexpr int kMonumentMinScore = 25;

class StationMonumentSystem : public ecs::System {
public:
    explicit StationMonumentSystem(ecs::World* world);
    ~StationMonumentSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "StationMonumentSystem"; }

    // Check player legend and create/upgrade a monument in the station if warranted.
    // Returns the monument entity id on creation/upgrade, or "" if no change.
    std::string checkAndCreateMonument(const std::string& station_entity_id,
                                       const std::string& player_entity_id,
                                       float timestamp);

    // Count monuments in a station
    int getMonumentCount(const std::string& station_entity_id) const;

    // Get type string for a player's monument in a station (or "None")
    std::string getMonumentType(const std::string& station_entity_id,
                                const std::string& player_id) const;

    // List all monument entity IDs in a station
    std::vector<std::string> getMonuments(const std::string& station_entity_id) const;

private:
    int monument_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STATION_MONUMENT_SYSTEM_H
