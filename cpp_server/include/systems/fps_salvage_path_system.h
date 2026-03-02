#ifndef NOVAFORGE_SYSTEMS_FPS_SALVAGE_PATH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FPS_SALVAGE_PATH_SYSTEM_H

#include "ecs/system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief FPS salvage path system (Phase 13)
 *
 * Manages FPS-mode salvage exploration including entry point cutting,
 * room exploration, and loot discovery/collection.
 */
class FPSSalvagePathSystem : public ecs::System {
public:
    explicit FPSSalvagePathSystem(ecs::World* world);
    ~FPSSalvagePathSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FPSSalvagePathSystem"; }

    // Initialization
    bool initializePath(const std::string& entity_id, const std::string& site_id,
                        const std::string& explorer_id, int total_rooms);

    // Entry point management
    bool addEntryPoint(const std::string& entity_id, const std::string& entry_id,
                       float cut_required, const std::string& tool_required);
    bool startCutting(const std::string& entity_id, const std::string& entry_id);

    // Exploration
    bool exploreRoom(const std::string& entity_id);
    bool setActive(const std::string& entity_id, bool active);

    // Loot management
    bool addLootNode(const std::string& entity_id, const std::string& loot_id,
                     const std::string& item_name,
                     components::FPSSalvagePath::LootRarity rarity, float value);
    bool discoverLoot(const std::string& entity_id, const std::string& loot_id);
    bool collectLoot(const std::string& entity_id, const std::string& loot_id);

    // Query
    float getExplorationProgress(const std::string& entity_id) const;
    int getDiscoveredLootCount(const std::string& entity_id) const;
    int getCollectedLootCount(const std::string& entity_id) const;
    std::string getEntryState(const std::string& entity_id, const std::string& entry_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FPS_SALVAGE_PATH_SYSTEM_H
