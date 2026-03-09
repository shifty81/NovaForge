#ifndef NOVAFORGE_SYSTEMS_OFFICER_SPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OFFICER_SPAWN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/npc_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Officer/Commander rare NPC spawn system
 *
 * Periodically evaluates spawn chance and creates rare officer NPCs
 * that carry faction or officer-grade loot.  Spawn chance increases
 * over time since the last officer was killed.
 */
class OfficerSpawnSystem : public ecs::SingleComponentSystem<components::OfficerSpawnState> {
public:
    explicit OfficerSpawnSystem(ecs::World* world);
    ~OfficerSpawnSystem() override = default;

    std::string getName() const override { return "OfficerSpawnSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& system_id = "");
    bool spawnOfficer(const std::string& entity_id, const std::string& npc_type,
                      components::OfficerSpawnState::OfficerRank rank =
                          components::OfficerSpawnState::OfficerRank::Commander);
    bool killOfficer(const std::string& entity_id, const std::string& officer_id);
    int  getAliveOfficerCount(const std::string& entity_id) const;
    int  getTotalSpawned(const std::string& entity_id) const;
    int  getTotalKilled(const std::string& entity_id) const;
    float getCurrentSpawnChance(const std::string& entity_id) const;
    float getTimeSinceLastSpawn(const std::string& entity_id) const;
    bool setActive(const std::string& entity_id, bool active);
    bool isActive(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::OfficerSpawnState& state,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OFFICER_SPAWN_SYSTEM_H
