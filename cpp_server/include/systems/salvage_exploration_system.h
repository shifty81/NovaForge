#ifndef NOVAFORGE_SYSTEMS_SALVAGE_EXPLORATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SALVAGE_EXPLORATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class SalvageExplorationSystem : public ecs::SingleComponentSystem<components::SalvageSite> {
public:
    explicit SalvageExplorationSystem(ecs::World* world);
    ~SalvageExplorationSystem() override = default;

    std::string getName() const override { return "SalvageExplorationSystem"; }

    bool discoverNode(const std::string& site_entity_id);
    bool lootNode(const std::string& site_entity_id);
    bool isFullyLooted(const std::string& site_entity_id) const;
    int getRemainingNodes(const std::string& site_entity_id) const;
    int getDiscoveredNodes(const std::string& site_entity_id) const;
    bool hasAncientTech(const std::string& site_entity_id) const;
    int generateTrinkets(const std::string& site_entity_id, uint64_t seed);

protected:
    void updateComponent(ecs::Entity& entity, components::SalvageSite& site, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SALVAGE_EXPLORATION_SYSTEM_H
