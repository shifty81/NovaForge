#include "systems/legend_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

LegendSystem::LegendSystem(ecs::World* world)
    : System(world) {
}

void LegendSystem::update(float /*delta_time*/) {
    // Legend system is event-driven via recordLegend
}

void LegendSystem::recordLegend(const std::string& entity_id,
                                 const std::string& type,
                                 const std::string& description,
                                 float timestamp,
                                 const std::string& system_id,
                                 int magnitude) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* legend = entity->getComponent<components::PlayerLegend>();
    if (!legend) return;
    legend->addEntry(type, description, timestamp, system_id, magnitude);
}

int LegendSystem::getLegendScore(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* legend = entity->getComponent<components::PlayerLegend>();
    if (!legend) return 0;
    return legend->legend_score;
}

std::string LegendSystem::computeTitle(int score) {
    if (score >= 500) return "Mythic";
    if (score >= 100) return "Legendary";
    if (score >= 50)  return "Famous";
    if (score >= 10)  return "Notable";
    return "Unknown";
}

std::string LegendSystem::getTitle(const std::string& entity_id) const {
    return computeTitle(getLegendScore(entity_id));
}

std::vector<components::PlayerLegend::LegendEntry>
LegendSystem::getLegendEntries(const std::string& entity_id, int count) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return {};
    auto* legend = entity->getComponent<components::PlayerLegend>();
    if (!legend) return {};

    int n = std::min(count, static_cast<int>(legend->entries.size()));
    return std::vector<components::PlayerLegend::LegendEntry>(
        legend->entries.end() - n, legend->entries.end());
}

} // namespace systems
} // namespace atlas
