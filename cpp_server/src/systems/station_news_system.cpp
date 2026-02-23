#include "systems/station_news_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

StationNewsSystem::StationNewsSystem(ecs::World* world)
    : System(world) {
}

void StationNewsSystem::update(float /*delta_time*/) {
    // News system is event-driven via report* methods
}

void StationNewsSystem::reportCombatEvent(const std::string& system_id,
                                           const std::string& details,
                                           float timestamp) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;
    auto* feed = entity->getComponent<components::StationNewsFeed>();
    if (!feed) return;
    feed->addEntry("Combat Alert", details, timestamp, "combat");
}

void StationNewsSystem::reportEconomyEvent(const std::string& system_id,
                                            const std::string& details,
                                            float timestamp) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;
    auto* feed = entity->getComponent<components::StationNewsFeed>();
    if (!feed) return;
    feed->addEntry("Economy Update", details, timestamp, "economy");
}

void StationNewsSystem::reportExplorationEvent(const std::string& system_id,
                                                const std::string& details,
                                                float timestamp) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;
    auto* feed = entity->getComponent<components::StationNewsFeed>();
    if (!feed) return;
    feed->addEntry("Exploration Discovery", details, timestamp, "exploration");
}

std::vector<components::StationNewsEntry>
StationNewsSystem::getNews(const std::string& system_id, int count) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return {};
    auto* feed = entity->getComponent<components::StationNewsFeed>();
    if (!feed) return {};

    int n = std::min(count, static_cast<int>(feed->entries.size()));
    return std::vector<components::StationNewsEntry>(
        feed->entries.end() - n, feed->entries.end());
}

int StationNewsSystem::getNewsCount(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0;
    auto* feed = entity->getComponent<components::StationNewsFeed>();
    if (!feed) return 0;
    return static_cast<int>(feed->entries.size());
}

} // namespace systems
} // namespace atlas
