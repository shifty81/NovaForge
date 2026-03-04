#ifndef NOVAFORGE_SYSTEMS_LEGEND_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LEGEND_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class LegendSystem : public ecs::SingleComponentSystem<components::PlayerLegend> {
public:
    explicit LegendSystem(ecs::World* world);
    ~LegendSystem() override = default;

    std::string getName() const override { return "LegendSystem"; }

    void recordLegend(const std::string& entity_id, const std::string& type,
                      const std::string& description, float timestamp,
                      const std::string& system_id, int magnitude);
    int getLegendScore(const std::string& entity_id) const;
    std::string getTitle(const std::string& entity_id) const;
    static std::string computeTitle(int score);
    std::vector<components::PlayerLegend::LegendEntry> getLegendEntries(
        const std::string& entity_id, int count = 20) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerLegend& legend, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LEGEND_SYSTEM_H
