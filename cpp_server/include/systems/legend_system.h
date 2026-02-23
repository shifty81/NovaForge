#ifndef EVE_SYSTEMS_LEGEND_SYSTEM_H
#define EVE_SYSTEMS_LEGEND_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class LegendSystem : public ecs::System {
public:
    explicit LegendSystem(ecs::World* world);
    ~LegendSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "LegendSystem"; }

    void recordLegend(const std::string& entity_id, const std::string& type,
                      const std::string& description, float timestamp,
                      const std::string& system_id, int magnitude);
    int getLegendScore(const std::string& entity_id) const;
    std::string getTitle(const std::string& entity_id) const;
    static std::string computeTitle(int score);
    std::vector<components::PlayerLegend::LegendEntry> getLegendEntries(
        const std::string& entity_id, int count = 20) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_LEGEND_SYSTEM_H
