#ifndef EVE_SYSTEMS_SALVAGE_EXPLORATION_SYSTEM_H
#define EVE_SYSTEMS_SALVAGE_EXPLORATION_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class SalvageExplorationSystem : public ecs::System {
public:
    explicit SalvageExplorationSystem(ecs::World* world);
    ~SalvageExplorationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SalvageExplorationSystem"; }

    bool discoverNode(const std::string& site_entity_id);
    bool lootNode(const std::string& site_entity_id);
    bool isFullyLooted(const std::string& site_entity_id) const;
    int getRemainingNodes(const std::string& site_entity_id) const;
    int getDiscoveredNodes(const std::string& site_entity_id) const;
    bool hasAncientTech(const std::string& site_entity_id) const;
    int generateTrinkets(const std::string& site_entity_id, uint64_t seed);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SALVAGE_EXPLORATION_SYSTEM_H
