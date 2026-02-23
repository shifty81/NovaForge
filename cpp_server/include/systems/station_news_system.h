#ifndef EVE_SYSTEMS_STATION_NEWS_SYSTEM_H
#define EVE_SYSTEMS_STATION_NEWS_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Generates station news entries from game events
 */
class StationNewsSystem : public ecs::System {
public:
    explicit StationNewsSystem(ecs::World* world);
    ~StationNewsSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "StationNewsSystem"; }

    // --- API ---
    void reportCombatEvent(const std::string& system_id, const std::string& details, float timestamp);
    void reportEconomyEvent(const std::string& system_id, const std::string& details, float timestamp);
    void reportExplorationEvent(const std::string& system_id, const std::string& details, float timestamp);
    std::vector<components::StationNewsEntry> getNews(const std::string& system_id, int count = 10) const;
    int getNewsCount(const std::string& system_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_STATION_NEWS_SYSTEM_H
