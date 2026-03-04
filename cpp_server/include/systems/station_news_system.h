#ifndef NOVAFORGE_SYSTEMS_STATION_NEWS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STATION_NEWS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Generates station news entries from game events
 */
class StationNewsSystem : public ecs::SingleComponentSystem<components::StationNewsFeed> {
public:
    explicit StationNewsSystem(ecs::World* world);
    ~StationNewsSystem() override = default;

    std::string getName() const override { return "StationNewsSystem"; }

    // --- API ---
    void reportCombatEvent(const std::string& system_id, const std::string& details, float timestamp);
    void reportEconomyEvent(const std::string& system_id, const std::string& details, float timestamp);
    void reportExplorationEvent(const std::string& system_id, const std::string& details, float timestamp);
    std::vector<components::StationNewsEntry> getNews(const std::string& system_id, int count = 10) const;
    int getNewsCount(const std::string& system_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::StationNewsFeed& feed, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STATION_NEWS_SYSTEM_H
